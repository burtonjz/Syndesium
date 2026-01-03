/*
 * Copyright (C) 2025 Jared Burton
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program. If not, see <https://www.gnu.org/licenses/>.
 */

#include "api/ApiHandler.hpp"
#include "core/Engine.hpp"
#include "config/Config.hpp"
#include "configs/ComponentConfig.hpp"
#include "meta/ComponentRegistry.hpp"
#include "types/ParameterType.hpp"
#include "types/SocketType.hpp"
#include "midi/MidiEventHandler.hpp"
#include "midi/MidiEventListener.hpp"

#include <netinet/in.h>
#include <string>
#include <sys/socket.h>
#include <fcntl.h>
#include <thread>
#include <optional>
#include <unordered_map>
#include <spdlog/spdlog.h>

ApiHandler::ApiHandler()
{}

ApiHandler* ApiHandler::instance(){
    static ApiHandler* s_instance = nullptr ;
    if ( !s_instance ){
        s_instance = new ApiHandler();
    }
    return s_instance ;
}

void ApiHandler::initialize(Engine* engine){
    engine_ = engine ;

    // register api handler functions
    handlers_["get_audio_devices"] = [this](int sock, const json& request){ return getAudioDevices(sock, request); };
    handlers_["get_midi_devices"] = [this](int sock, const json& request){ return getMidiDevices(sock, request); };
    handlers_["set_audio_device"] = [this](int sock, const json& request){ return setAudioDevice(sock, request); };
    handlers_["set_midi_device"] = [this](int sock, const json& request){ return setMidiDevice(sock, request); };
    handlers_["set_state"] = [this](int sock, const json& request){ return setState(sock, request); };
    handlers_["get_configuration"] = [this](int sock, const json& request){ return getConfiguration(sock, request); };
    handlers_["load_configuration"] = [this](int sock, const json& request){ return loadConfiguration(sock, request); };
    handlers_["get_waveforms"] = [this](int sock, const json& request){ return getWaveforms(sock, request); };
    handlers_["add_component"] = [this](int sock, const json& request){ return addComponent(sock, request); };
    handlers_["remove_component"] = [this](int sock, const json& request){ return removeComponent(sock, request); };
    handlers_["get_component_parameter"] = [this](int sock, const json& request){ return getComponentParameter(sock, request); };
    handlers_["set_component_parameter"] = [this](int sock, const json& request){ return setComponentParameter(sock, request); };
    handlers_["create_connection"] = [this](int sock, const json& request){ return createConnection(sock, request); };
    handlers_["remove_connection"] = [this](int sock, const json& request){ return removeConnection(sock, request); };
    handlers_["get_sequence"] = [this](int sock, const json& request){ return getSequence(sock, request); };
    handlers_["add_sequence_note"] = [this](int sock, const json& request){ return addSequenceNote(sock, request); };
    handlers_["remove_sequence_note"] = [this](int sock, const json& request){ return removeSequenceNote(sock, request); };
}

void ApiHandler::start(){
    int serverPort = Config::get<int>("server.port").value() ;

    // Create socket
    int serverSock = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSock == -1) {
        SPDLOG_WARN("Socket creation failed");
        exit(1);
    }
    int opt = 1 ;
    if (setsockopt(serverSock, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0){
        SPDLOG_WARN("setsockopt(SO_REUSEADDR) failed");
        exit(1);
    }

    // set socket as nonblocking
    fcntl(serverSock, F_SETFL, O_NONBLOCK);

    sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = INADDR_ANY;
    serverAddr.sin_port = htons(serverPort);

    // Bind socket to address
    if (bind(serverSock, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) < 0) {
        SPDLOG_WARN("Bind failed");
        exit(1);
    }

    // Listen for incoming connections
    if (listen(serverSock, 3) < 0) {
        SPDLOG_WARN("Listen failed");
        return;
    }

    SPDLOG_INFO("Server listening on port {}... ",serverPort);

    // Accept incoming client connections in a loop
    while (!Engine::stop_flag) {
        int sock = accept(serverSock, nullptr, nullptr);
        if (sock >= 0) {
            // set client socket to non-blocking
            int flags = fcntl(sock, F_GETFL, 0);
            if ( flags == -1 ){
                perror("fcntl F_GETFL");
                close(sock);
                continue ;
            }
            if ( fcntl(sock, F_SETFL, flags | O_NONBLOCK ) == -1 ){
                perror("fcntl F_SETFL");
                close(sock);
                continue ;
            }

            // Handle client in a separate thread
            std::thread([sock](){
                ApiHandler::instance()->onClientConnection(sock);
            }).detach();  
        } else {
            // no pending connection; sleep briefly to avoid busy-wait
            std::this_thread::sleep_for(std::chrono::milliseconds(1000));
        }
    }

    // Close the server socket
    close(serverSock);
}

void ApiHandler::onClientConnection(int sock){
    char buffer[1024] = {0};
    std::string partialData ;

    while (!Engine::stop_flag){
        ssize_t bytesReceived = recv(sock, buffer, sizeof(buffer) - 1, 0) ;
        if (bytesReceived > 0 ){
            buffer[bytesReceived] = '\0' ; // null-terminate the buffer so it can be read into a string
            partialData += buffer ; // read buffered text into data string

            // loop until we find a newline character
            size_t pos ;
            while ((pos = partialData.find('\n')) != std::string::npos ){
                // copy a complete message to our string to parse, then erase that portion from our data string.
                std::string jsonStr = partialData.substr(0,pos); 
                partialData.erase(0, pos + 1); 

                // handle the parsed json string
                handleClientMessage(sock, jsonStr);
            }
        } else if (bytesReceived == 0){
            break ;
        } else {
            if (errno == EAGAIN || errno == EWOULDBLOCK ){
                std::this_thread::sleep_for(std::chrono::milliseconds(50));
                continue ;
            } else {
                perror("recv");
                break ;
            }
        }
    }

    close(sock);
}

json ApiHandler::sendApiResponse(int sock, json& response, const std::string& err){
    if ( err == "" ){
        response["status"] = "success" ;
    } else {
        response["status"] = "failed" ;
        response["error"] = err ;
    }
    std::string r = response.dump() + '\n' ;
    SPDLOG_INFO("sending API response: {}", r.c_str());
    send(sock, r.c_str(), r.size(), 0);
    return response ;
}


void ApiHandler::handleClientMessage(int sock, std::string jsonStr){
    json request;
    std::string action ;

    SPDLOG_INFO("received request: {}", jsonStr);

    try {
        request = json::parse(jsonStr);
        action = request["action"];
    } catch (const std::exception& e){
        sendApiResponse(sock,request, "Error parsing json request: " + std::string(e.what()));
        return ;
    }
    
    auto it = handlers_.find(action);
    if ( it == handlers_.end() ){
        sendApiResponse(sock, request, "unknown action requested: " + action );
        return ;
    }
    
    it->second(sock, request);
}

json ApiHandler::getAudioDevices(int sock, const json& request){
    json response = request ;
    response["data"] = engine_->getAvailableAudioDevices() ;
    return sendApiResponse(sock,response);
}

json ApiHandler::getMidiDevices(int sock, const json& request){
    json response = request ;
    response["data"] = engine_->getAvailableMidiDevices() ;
    return sendApiResponse(sock,response);
}

json ApiHandler::setAudioDevice(int sock, const json& request){
    json response = request ;
    int deviceId ;
    std::string err ;
    
    try {
        deviceId = response["device_id"];
    } catch (const std::exception& e){
        return sendApiResponse(sock,response, "Error parsing json request: " + std::string(e.what()) );
    }

    if ( engine_->setAudioDeviceId(deviceId) ){
        return sendApiResponse(sock,response);
    } else {
        return sendApiResponse(sock, response, "failed to set audio device");
    }

    return response ;
}

json ApiHandler::setMidiDevice(int sock, const json& request){
    json response = request ;
    int deviceId ;
    
    try {
        deviceId = response["device_id"];
    } catch (const std::exception& e){
        return sendApiResponse(sock,response, "Error parsing json request: " + std::string(e.what()) );
    }

    if ( engine_->setMidiDeviceId(deviceId) ){
        return sendApiResponse(sock,response);
    } else {
        return sendApiResponse(sock, response, "failed to set midi device");
    }
}

json ApiHandler::setState(int sock, const json& request){
    json response = request ;
    std::string state ;

    try {
        state = response["state"];
    } catch (const std::exception& e){
        return sendApiResponse(sock,response, "Error parsing json request: " + std::string(e.what()) );
    }

    if ( state == "run" ){
        engine_->run();
        return sendApiResponse(sock,response);
    }

    if ( state == "stop" ){
        engine_->stop();
        return sendApiResponse(sock,response);
    }

    return sendApiResponse(sock,response, "Unrecognized engine state requested: " + state);
}

json ApiHandler::getConfiguration(int sock, const json& request){
    json response = request ;
    response["data"] = engine_->serialize();
    return sendApiResponse(sock,response);
}

json ApiHandler::loadConfiguration(int sock, const json& request){
    json response = request ;
    
    // create components
    try {
        response.at("components");
        assert(response["components"].is_array() && "components is not a json array");
    } catch ( const std::exception& e ){
        return sendApiResponse(sock, response, "Error processing json request " + std::string(e.what()));
    }

    std::unordered_map<int,int> idMap ;
    if ( ! loadCreateComponent(sock,response["components"], idMap) ){
        return sendApiResponse(sock, response, "Error creating components");
    }

    // update response data with new component ids
    loadUpdateIds(response, idMap);

    // connect components
    if ( ! loadConnectComponent(sock, response) ){
        return sendApiResponse(sock, response, "Error connecting components");
    }

    return sendApiResponse(sock, response);
}

json ApiHandler::getWaveforms(int sock, const json& request){
    json response = request ;
    response["data"] = Waveform::getWaveforms() ;
    return sendApiResponse(sock,response);
}

json ApiHandler::addComponent(int sock, const json& request){
    json response = request ;
    ComponentType type ;
    std::string name ;

    try {
        type = static_cast<ComponentType>(response["type"]);
        name = response["name"];
    } catch (const std::exception& e){
        return sendApiResponse(sock,response, "Error parsing json request: " + std::string(e.what()) );
    }

    ComponentId id = engine_->componentFactory.createFromJson(type, name, getDefaultConfig(type));
    response["componentId"] = id;
    return sendApiResponse(sock,response);
}

json ApiHandler::removeComponent(int sock, const json& request){
    json response = request ;
    ComponentId id ;

    try {
        id = response["componentId"];
    } catch (const std::exception& e){
        return sendApiResponse(sock,response, "Error parsing json request: " + std::string(e.what()) );
    }
    
    auto c = engine_->componentManager.getRaw(id);
    if ( !c ){
        return sendApiResponse(sock, response, "component not found.");
    }

    // query each subsystem and remove connections, if exist
    bool allRemoved = true ;

    auto connections = engine_->getComponentMidiConnections(id);
    SPDLOG_DEBUG("removing midi connections from component with id {}", id);
    for ( auto c : connections ){
        c.remove = true ;
        json j = c ;
        SPDLOG_DEBUG("removing midi connection: {}", j.dump());
        auto cresponse = removeConnection(sock, j);
        allRemoved = allRemoved && cresponse.contains("status") && cresponse["status"] == "success" ;
    }

    connections = engine_->getComponentSignalConnections(id);
    SPDLOG_DEBUG("removing audio connections from component with id {}", id);
    for ( auto c : connections ){
        c.remove = true ;
        json j = c ;
        SPDLOG_DEBUG("removing audio connection: {}", j.dump());
        auto cresponse = removeConnection(sock, j);
        allRemoved = allRemoved && cresponse.contains("status") && cresponse["status"] == "success" ;
    }

    
    connections = engine_->getComponentModulationConnections(id);
    SPDLOG_DEBUG("removing modulation connections from component with id {}", id);
    for ( auto c : connections ){
        c.remove = true ;
        json j = c ;
        SPDLOG_DEBUG("removing modulation connection: {}", j.dump());
        auto cresponse = removeConnection(sock, j);
        allRemoved = allRemoved && cresponse.contains("status") && cresponse["status"] == "success" ;
    }

    if ( !allRemoved ){
        return sendApiResponse(sock, response, "at least one component connection could not be removed.");
    }

    engine_->componentManager.remove(id);
    return sendApiResponse(sock, response);    
}

json ApiHandler::getComponentParameter(int sock, const json& request){
    json response = request ;
    return sendApiResponse(sock, response, "get component parameter not yet implemented");
}

json ApiHandler::setComponentParameter(int sock, const json& request){
    json response = request ;
    ComponentId id ;
    ParameterType param ;

    try {
        id = response["componentId"];
        param = static_cast<ParameterType>(response["parameter"]);
        response.at("value"); // verify present
    } catch (const std::exception& e){
        return sendApiResponse(sock,response, "Error parsing json request: " + std::string(e.what()) );
    }


    if ( engine_->componentManager.setComponentParameter(id, param, response["value"]) ){
        return sendApiResponse(sock,response);
    } else {
        return sendApiResponse(sock,response, "Error setting component parameter." );
    }
}

json ApiHandler::createConnection(int sock, const json& request){
    json response = request ;

    try {
        response.at("inbound");
        response.at("outbound");
    } catch (const std::exception& e){
        return sendApiResponse(sock,response, "Error parsing json request: " + std::string(e.what()) );
    }

    ConnectionRequest req = response.get<ConnectionRequest>() ;
    if ( routeConnectionRequest(req)){
        return sendApiResponse(sock,response);
    } else {
        return sendApiResponse(sock,response, "failed to make requested connection");
    }
}

json ApiHandler::removeConnection(int sock, const json& request){
    json response = request ;

    try {
        response.at("inbound");
        response.at("outbound");
    } catch (const std::exception& e){
        return sendApiResponse(sock,response, "Error parsing json request: " + std::string(e.what()) );
    }

    ConnectionRequest req = response.get<ConnectionRequest>() ;
    req.remove = true ;

    if ( routeConnectionRequest(req)){
        return sendApiResponse(sock,response);
    } else {
        return sendApiResponse(sock,response, "failed to make requested connection");
    }
}

json ApiHandler::getSequence( int sock, const json& request){
    json response = request ;
    ComponentId id ;

    try {
        id = response["componentId"];
    } catch ( const std::exception& e ){
        return sendApiResponse(sock,response, "Error parsing json request: " + std::string(e.what()) );
    }

    MidiEventHandler* component = engine_->componentManager.getMidiHandler(id);
    if ( !component || !component->hasSequence() ){
        return sendApiResponse(sock, response, "No sequenceable component matching id" + std::to_string(id));
    }

    response["sequence"] = component->getSequence().get();
    return response ;
}

json ApiHandler::addSequenceNote( int sock, const json& request){
    json response = request ;
    ComponentId id ;
    SequenceNote n ;

    try {
        id = response["componentId"];
        n = response["note"];
    } catch ( const std::exception& e ){
        return sendApiResponse(sock,response, "Error parsing json request: " + std::string(e.what()) );
    }

    MidiEventHandler* component = engine_->componentManager.getMidiHandler(id);
    if ( !component || !component->hasSequence() ){
        return sendApiResponse(sock, response, "No sequenceable component matching id" + std::to_string(id));
    }

    if ( component->addSequenceNote(n) ){
        return sendApiResponse(sock, response);
    }

    return sendApiResponse(sock, response, "failed to add sequence note.");

}

json ApiHandler::removeSequenceNote( int sock, const json& request){
    json response = request ;
    ComponentId id ;
    SequenceNote n ;

    try {
        id = response["componentId"];
        n = response["note"];
    } catch ( const std::exception& e ){
        return sendApiResponse(sock,response, "Error parsing json request: " + std::string(e.what()) );
    }

    MidiEventHandler* component = engine_->componentManager.getMidiHandler(id);
    if ( !component || !component->hasSequence() ){
        return sendApiResponse(sock, response, "No sequenceable component matching id" + std::to_string(id));
    }

    if ( component->removeSequenceNote(n) ){
        return sendApiResponse(sock, response);
    }

    return sendApiResponse(sock, response, "failed to add sequence note.");
}

bool ApiHandler::routeConnectionRequest(ConnectionRequest request){
    if ( request.inboundSocket == SocketType::MidiInbound && request.outboundSocket == SocketType::MidiOutbound )
        return engine_->handleMidiConnection(request);
    if ( request.inboundSocket == SocketType::SignalInbound && request.outboundSocket == SocketType::SignalOutbound )
        return engine_->handleSignalConnection(request);

    if ( request.inboundSocket == SocketType::ModulationInbound && request.outboundSocket == SocketType::ModulationOutbound )
        return engine_->handleModulationConnection(request);

    SPDLOG_WARN("WARN: socket types are incompatible. No connection will be made");
    return false ;
}

bool ApiHandler::loadCreateComponent(int sock, const json& components, std::unordered_map<int,int>& idMap){
    json params ;
    ComponentId id ;
    ComponentType type ;
    json componentRequest ;
    json componentResponse ;
    json parameterRequest ;
    ParameterType parameterType ;
    for ( const auto& component : components ){
        try {
            params = component["parameters"];
            id = component["id"];
            type = static_cast<ComponentType>(component["type"]);
            componentRequest["action"] = "add_component" ;
            componentRequest["name"] = ComponentRegistry::getComponentDescriptor(type).name ;
            componentRequest["type"] = static_cast<int>(type) ;
            componentResponse = addComponent(sock, componentRequest);
            idMap[id] = componentResponse["componentId"];

            for ( const auto& [p, data] : params.items() ){
                if ( ! data.contains("currentValue") ){
                    continue ; // just has modulation
                }
                parameterType = static_cast<ParameterType>(parameterFromString(p));
                parameterRequest["action"] = "set_component_parameter" ;
                parameterRequest["componentId"] = idMap[id] ;
                parameterRequest["parameter"] = static_cast<int>(parameterType);
                parameterRequest["value"] = data["currentValue"] ;
                setComponentParameter(sock, parameterRequest);
            }
        } catch ( const std::exception& e ){
            SPDLOG_WARN("Error creating component: {}", std::string(e.what()));
            return false ;
        }
    }
    return true ;
}

bool ApiHandler::loadConnectComponent(int sock, const json& config){
    // first handle audio sinks
    json audioSinks ;
    try {
        assert(config["AudioSinks"].is_array() && "'AudioSinks' json data is not in expected format");
    } catch (const std::exception& e){
        SPDLOG_WARN("Error processing json request: {}", std::string(e.what()));
        return false ;
    }

    json request ;
    json connectionResponse ;
    json inbound ;
    json outbound ;
    for ( const auto& id : config["AudioSinks"] ){
        request["action"] = "create_connection" ;
        inbound["socketType"] = SocketType::SignalInbound ;
        outbound["socketType"] = SocketType::SignalOutbound ;
        outbound["id"] = id ;
        
        request["inbound"] = inbound ;
        request["outbound"] = outbound ;
        connectionResponse = createConnection(sock, request);
        if ( ! connectionResponse.contains("status") || connectionResponse["status"] != "success" ){
            SPDLOG_WARN("error requesting connection: {}", connectionResponse.dump());
            return false ;
        }

        request.clear();
        inbound.clear();
        outbound.clear();
    }

    // now rootMidiHandlers
    json midiHandlers ;
    try {
        assert(config["rootMidiHandlers"].is_array() && "'rootMidiHandlers' json data is not in expected format");
    } catch (const std::exception& e){
        SPDLOG_WARN("Error processing json request: {}", std::string(e.what()));
        return false ;
    }

    for ( const auto& id : config["rootMidiHandlers"] ){
        request["action"] = "create_connection" ;
        inbound["socketType"] = SocketType::MidiInbound ;
        outbound["socketType"] = SocketType::MidiOutbound ;
        inbound["id"] = id ;

        request["inbound"] = inbound ;
        request["outbound"] = outbound ;

        connectionResponse = createConnection(sock, request);
        if ( ! connectionResponse.contains("status") || connectionResponse["status"] != "success" ){
            SPDLOG_WARN("error requestion connection: {}", connectionResponse.dump());
            return false ;
        }

        request.clear();
        inbound.clear();
        outbound.clear();
    }

    // lastly, component to component connections
    try {
        assert(config["components"].is_array() && "'components' json data is not in expected format");
    } catch (const std::exception& e){
        SPDLOG_WARN("Error processing json request: {}", std::string(e.what())) ;
        return false ;
    }

    json params ;
    json midiListeners ;
    json signalInputs ;
    ComponentId id ;

    
    for ( const auto& component : config["components"] ){
        if ( ! component.is_object() ){
            SPDLOG_WARN("component is not in expected format: {}", component.dump()) ;
            return false ;
        }

        try {
            id = component["id"] ;
            params = component["parameters"];
            assert(params.is_object() && "parameters is not in the correct format.");
        } catch (const std::exception& e){
            SPDLOG_ERROR("Error processing json components object: {}", std::string(e.what()));
            return false ;
        } 

        // audio connections
        if ( component.contains("signalInputs") && component["signalInputs"].is_array() ){
            for ( const auto& outboundId : component["signalInputs"] ) {
                request["action"] = "create_connection" ;

                inbound["id"] = id ;
                inbound["socketType"] = SocketType::SignalInbound ;

                outbound["id"] = outboundId ;
                outbound["socketType"] = SocketType::SignalOutbound ;

                request["inbound"] = inbound ;
                request["outbound"] = outbound ;

                connectionResponse = createConnection(sock, request);
                if ( ! connectionResponse.contains("status") || connectionResponse["status"] != "success" ){
                    SPDLOG_WARN("error requesting connection: {}", connectionResponse.dump());
                    return false ;
                }

                request.clear();
                inbound.clear();
                outbound.clear();
            }
        }

        // midi connections
        if ( component.contains("midiListeners") && component["midiListeners"].is_array()){
            for ( const auto& inboundId : component["midiListeners"] ){
                request["action"] = "create_connection" ;

                inbound["id"] = inboundId ;
                inbound["socketType"] = SocketType::MidiInbound ;
                
                outbound["id"] = id ;
                outbound["socketType"] = SocketType::MidiOutbound ;

                request["inbound"] = inbound ;
                request["outbound"] = outbound ;

                connectionResponse = createConnection(sock, request);
                if ( ! connectionResponse.contains("status") || connectionResponse["status"] != "success" ){
                    SPDLOG_WARN("error requesting connection: {}", connectionResponse.dump());
                    return false ;
                }

                request.clear();
                inbound.clear();
                outbound.clear();
            }
        }

        // modulation connections
        for ( const auto& [p, data] : params.items() ){
            if ( data.contains("modulatorId") ){
                request["action"] = "create_connection" ;

                inbound["id"] = id ;
                inbound["socketType"] = SocketType::ModulationInbound ;
                inbound["parameter"] = parameterFromString(p);

                outbound["id"] = data["modulatorId"];
                outbound["socketType"] = SocketType::ModulationOutbound ;
                
                request["inbound"] = inbound ;
                request["outbound"] = outbound ;

                connectionResponse = createConnection(sock, request);
                if ( ! connectionResponse.contains("status") || connectionResponse["status"] != "success" ){
                    SPDLOG_WARN("error requesting connection: {}", connectionResponse.dump());
                    return false ;
                }

                request.clear();
                inbound.clear();
                outbound.clear();
            }
        }
    }

    return true ;

}


void ApiHandler::loadUpdateIds(json& j, const std::unordered_map<int, int>& idMap){
    std::vector<std::string> keys = {"id", "ComponentId", "signalInputs", "rootMidiHandlers", "midiListeners", "modulatorId", "AudioSinks"};
    if ( j.is_object() ) {
        for ( const auto& key : keys ) {
            // single values
            if ( j.contains(key) && j[key].is_number_integer() ) {
                int currentId = j[key];
                auto it = idMap.find(currentId);
                if ( it != idMap.end() ) {
                    j[key] = it->second; 
                }
            }

            // array values
            if (j.contains(key) && j[key].is_array()) {
                for (auto& element : j[key]) {
                    if (element.is_number_integer()) {
                        int currentId = element;
                        auto it = idMap.find(currentId);
                        if (it != idMap.end()) {
                            element = it->second;
                        }
                    }
                }
            }
        }
        
        // recurse nested values
        for ( auto& [key, value] : j.items() ) {
            loadUpdateIds(value, idMap);
        }
    }
    else if ( j.is_array() ) {
        // recurse array elements
        for (auto& element : j) {
            loadUpdateIds(element, idMap);
        }
    }
}