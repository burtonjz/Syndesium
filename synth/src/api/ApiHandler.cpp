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
#include "configs/ModulatorConfig.hpp"
#include "configs/ModuleConfig.hpp"
#include "meta/ComponentDescriptor.hpp"
#include "types/SocketType.hpp"

#include <iostream>
#include <netinet/in.h>
#include <sys/socket.h>
#include <fcntl.h>
#include <thread>
#include <optional>

void ApiHandler::start(Engine* engine){
    int serverPort = Config::get<int>("server.port").value() ;

    // Create socket
    int serverSock = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSock == -1) {
        std::cerr << "Socket creation failed" << std::endl ;
        exit(1);
    }
    int opt = 1 ;
    if (setsockopt(serverSock, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0){
        std::cerr << "setsockopt(SO_REUSEADDR) failed" << std::endl ;
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
        std::cerr << "Bind failed" << std::endl ;
        exit(1);
    }

    // Listen for incoming connections
    if (listen(serverSock, 3) < 0) {
        std::cerr << "Listen failed" << std::endl;
        return;
    }

    std::cout << "Server listening on port " << serverPort << "..." << std::endl;

    // Accept incoming client connections in a loop
    while (!Engine::stop_flag) {
        int clientSock = accept(serverSock, nullptr, nullptr);
        if (clientSock >= 0) {
            // set client socket to non-blocking
            int flags = fcntl(clientSock, F_GETFL, 0);
            if ( flags == -1 ){
                perror("fcntl F_GETFL");
                close(clientSock);
                continue ;
            }
            if ( fcntl(clientSock, F_SETFL, flags | O_NONBLOCK ) == -1 ){
                perror("fcntl F_SETFL");
                close(clientSock);
                continue ;
            }

            // Handle client in a separate thread
            std::thread(onClientConnection, engine, clientSock).detach();  
        } else {
            // no pending connection; sleep briefly to avoid busy-wait
            std::this_thread::sleep_for(std::chrono::milliseconds(1000));
        }
    }

    // Close the server socket
    close(serverSock);
}

void ApiHandler::onClientConnection(Engine* engine, int clientSock){
    char buffer[1024] = {0};
    std::string partialData ;

    while (!Engine::stop_flag){
        ssize_t bytesReceived = recv(clientSock, buffer, sizeof(buffer) - 1, 0) ;
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
                handleClientMessage(engine, clientSock, jsonStr);
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

    close(clientSock);
}

void ApiHandler::sendApiResponse(int clientSock, json response){
    std::string r = response.dump() + '\n' ;
    std::cout << "sending API response: " << r.c_str() << std::endl  ;
    send(clientSock, r.c_str(), r.size(), 0);
}

void ApiHandler::handleClientMessage(Engine* engine, int clientSock, std::string jsonStr){
    json jResponse ;
    json jRequest ;

    std::string err ;
    auto sendError = [&](const std::string& message){
        jResponse["status"] = "failed" ;
        jResponse["error"] = message ;
        sendApiResponse(clientSock, jResponse);
    };

    auto sendSuccess = [&](){
        jResponse["status"] = "success" ;
        sendApiResponse(clientSock, jResponse);
    };

    try {
        jRequest = json::parse(jsonStr);

        std::string action = jRequest["action"] ;
        jResponse["action"] = action ;

        std::cout << "received request for action: " << action << std::endl ;

        if ( action == "get_audio_devices" ) {
            jResponse["data"] = engine->getAvailableAudioDevices() ;
            sendSuccess();
            return ;
        }

        if ( action == "get_midi_devices" ){
            jResponse["data"] = engine->getAvailableMidiDevices() ;
            sendSuccess();
            return ;
        }

        if ( action == "set_audio_device" ){
            int deviceId = jRequest["device_id"] ;
            engine->setAudioDeviceId(deviceId);
            sendSuccess();
            return ;
        } 
        
        if ( action == "set_midi_device" ){
            int deviceId = jRequest["device_id"] ;
            engine->setMidiDeviceId(deviceId);
            sendSuccess();
            return ;
        }

        if ( action == "set_state" ){
            jResponse["state"] = jRequest["state"] ;
            if ( jResponse["state"] == "run" ){
                std::cout << "Running engine. " << std::endl ;
                engine->run();
                sendSuccess();
                return ;
            } 
            
            if ( jResponse["state"] == "stop" ){
                std::cout << "Stopping engine. " << std::endl ;
                engine->stop();
                sendSuccess();
                return ;
            } 
            
            err = "Unrecognized engine state requested: " + jResponse["state"].get<std::string>() ;
            sendError(err);
            return ;
        }

        if ( action == "get_waveforms" ){
            jResponse["data"] = Waveform::getWaveforms() ;
            sendSuccess();
            return ;
        }
        // COMPONENTS
        if ( action == "add_component"){
            jResponse["is_module"] = jRequest["is_module"] ;
            jResponse["type"] = jRequest["type"] ;
            jResponse["name"] = jRequest["name"] ;
            if( jResponse["is_module"] ){
                ModuleType type = static_cast<ModuleType>(jRequest["type"]);
                ComponentId id = engine->moduleController.dispatchFromJson(
                    type,
                    jRequest["name"], 
                    Module::getDefaultConfig(type)
                );
                jResponse["component_id"] = id ;
                sendSuccess();
                return ;
            } else {
                ModulatorType type = static_cast<ModulatorType>(jRequest["type"]);
                ModulatorID id = engine->modulationController.dispatchFromJson(
                    type,
                    jRequest["name"],
                    Modulator::getDefaultConfig(type)
                );
                jResponse["component_id"] = id ;
                sendSuccess();
                return ;
            }
        }

        if ( action == "set_component_parameter"){
            int componentId = jRequest["componentId"];
            ParameterType p = static_cast<ParameterType>(jRequest["parameter"]);
            ParameterValue val = json2Variant(jRequest["value"]);
            bool isModule = jRequest["isModule"];

            if ( isModule ){
                jRequest["status"] = engine->moduleController.setComponentParameter(componentId, p, val);
            } else {
                jRequest["status"] = engine->modulationController.setComponentParameter(componentId, p, val);
            }

            if ( jRequest["status"] ){
                sendSuccess();
                return ;
            } else {
                sendError("Error setting component parameter.");
                return ;
            }
        }

        if ( action == "get_module_parameter" ){
            int moduleID = jRequest["id"];
            ParameterType p = static_cast<ParameterType>(jRequest["parameter"]);
            BaseModule* module = engine->moduleController.getRaw(moduleID);
            if  (!module){
                err = "Unable to find modulator of with ID" + std::to_string(moduleID) ;
                sendError(err);
                return ;
            }
            auto params = module->getParameters()->getBaseMap() ;
            auto it = params.find(p);
            if (it == params.end()){
                err = "ParameterType " + parameter2String(p) + "is not present in map for ModuleID " + std::to_string(moduleID) ; 
                sendError(err);
                return ;
            }
            jRequest["data"] = ParameterMap::dispatchToJson(module->getParameters()->getValueDispatch(p));
            sendSuccess();
            return ;
        }

        if ( action == "get_modulator_parameter" ){
            int modulatorID = jRequest["id"];
            ParameterType p = static_cast<ParameterType>(jRequest["parameter"]);
            BaseModulator* modulator = engine->modulationController.getRaw(modulatorID);
            if (!modulator){
                err = "Unable to find modulator of with ID" + std::to_string(modulatorID) ;
                sendError(err);
                return ;
            }
            auto params = modulator->getParameters()->getBaseMap() ;
            auto it = params.find(p);
            if (it == params.end()){
                err = "ParameterType " + parameter2String(p) + " is not present in map for ModulatorID " + std::to_string(modulatorID) ; 
                sendError(err);
                return ;
            }
            jRequest["data"] = ParameterMap::dispatchToJson(modulator->getParameters()->getValueDispatch(p));
            sendSuccess();
            return ;
        }

        if ( action == "create_connection" ){
            // save variables into response
            jResponse["input"] = jRequest["input"] ;
            jResponse["output"] = jRequest["output"] ;

            ConnectionRequest request = parseConnectionRequest(jRequest);
            if ( routeConnectionRequest(engine, request)){
                sendSuccess();
                return ;
            } else {
                sendError("failed to make requested connection");
                return ;
            }
        }

        err = "Unknown action requested: " + action ;
        sendError(err);

    } catch (const std::exception& e){
        err = "JSON parse error: " + std::string(e.what()) ;
        sendError(err);
    }
}

ConnectionRequest ApiHandler::parseConnectionRequest(json request){
    ConnectionRequest req ;
    // define API variables
    req.inboundSocket = static_cast<SocketType>(request["input"]["socket"]);
    req.outboundSocket = static_cast<SocketType>(request["output"]["socket"]);
    if ( request["input"].contains("id")) req.inboundID = request["input"]["id"];
    if ( request["output"].contains("id")) req.outboundID = request["output"]["id"];
    if ( request["input"].contains("is_module")) req.inboundIsModule = request["input"]["is_module"];
    if ( request["output"].contains("is_module")) req.outboundIsModule = request["output"]["is_module"];
    if ( request["input"].contains("parameter")) req.inboundParameter = static_cast<ParameterType>(request["input"]["parameter"]);

    return req ;
}

bool ApiHandler::routeConnectionRequest(Engine* engine, ConnectionRequest request){
    if ( request.inboundSocket == SocketType::MidiInput && request.outboundSocket == SocketType::MidiOutput )
        return handleMidiConnection(engine, request);
    
    if ( request.inboundSocket == SocketType::SignalInput && request.outboundSocket == SocketType::SignalOutput )
        return handleSignalConnection(engine, request);

    if ( request.inboundSocket == SocketType::ModulationInput && request.outboundSocket == SocketType::ModulationOutput )
        return handleModulationConnection(engine, request);

    std::cerr << "WARN: socket types are incompatible. No connection will be made" << std::endl ;
    return false ;
}

bool ApiHandler::handleSignalConnection(Engine* engine, ConnectionRequest request){
    BaseModule* inbound = nullptr ;
    BaseModule* outbound = nullptr ;

    inbound = engine->moduleController.getRaw(request.inboundID.value_or(-1));
    outbound = engine->moduleController.getRaw(request.outboundID.value_or(-1));
    
    
    // if the source is an external endpoint
    if ( ! request.outboundID.has_value() ){
        std::cerr << "receiving audio from an input device is not yet supported." << std::endl ;
        return false ;
    }

    // if the destination is an external endpoint
    if ( ! request.inboundID.has_value() ){
        engine->moduleController.registerSink(outbound);
        return true ;
    }
    
    engine->moduleController.connect(outbound, inbound);
    return true ;
}

bool ApiHandler::handleMidiConnection(Engine* engine, ConnectionRequest request){
    if ( ! request.outboundID.has_value() && ! request.inboundID.has_value() ){
        std::cerr << "WARN: midi connection was requested with an invalid object (no id supplied for inbound or outbound objects)." << std::endl ;
        return false ;
    }

    if ( request.outboundID.has_value() && request.inboundID.has_value() ){
        // this is a standard connection between a handler and a listener
        MidiEventHandler* handler ;
        MidiEventListener* listener ;

        if ( request.inboundIsModule.value() ){
            listener = dynamic_cast<MidiEventListener*>(engine->moduleController.getRaw(request.inboundID.value()));
        } else {
            listener = dynamic_cast<MidiEventListener*>(engine->modulationController.getRaw(request.inboundID.value()));
        }

        if ( !listener ){
            std::cerr << "WARN: No valid MidiEventListener found from connection request configuration" << std::endl ;
            return false ;
        }

        if ( request.outboundIsModule.value() ){
            handler = dynamic_cast<MidiEventHandler*>(engine->moduleController.getRaw(request.outboundID.value()));
        } else {
            handler = dynamic_cast<MidiEventHandler*>(engine->modulationController.getRaw(request.outboundID.value()));
        }
        
        if ( !handler ){
            std::cerr << "WARN: No valid MidiEventHandler found from connection request configuration" << std::endl ;
            return false ;
        }

        return engine->setMidiConnection(handler, listener);    
    }

    if ( ! request.outboundID.has_value() && request.inboundID.has_value() ){
        // if we are connecting from a outbound system MIDI
        // this is most common if we are connecting the raw MIDI to a MidiEventHandler
        // but it can also connect straight to a listener if desired
        MidiEventHandler* inboundHandler ;
        MidiEventListener* inboundListener ;

        // and it can be cast from either a module or modulator
        if ( request.inboundIsModule.value() ){
            inboundHandler = dynamic_cast<MidiEventHandler*>(engine->moduleController.getRaw(request.inboundID.value()));
            inboundListener = dynamic_cast<MidiEventListener*>(engine->moduleController.getRaw(request.inboundID.value()));
        } else {
            inboundHandler = dynamic_cast<MidiEventHandler*>(engine->modulationController.getRaw(request.inboundID.value()));
            inboundListener = dynamic_cast<MidiEventListener*>(engine->modulationController.getRaw(request.inboundID.value()));
        }

        if (inboundHandler){
            // we register the handler to our midi state
            engine->registerMidiHandler(inboundHandler);
            return true ;
        }

        if (inboundListener){
            // we register the listener to the default handler
            engine->setMidiConnection(nullptr, inboundListener);
            return true ;
        }

        std::cerr << "WARN: midi connection was requested with an invalid object." << std::endl ;
        return false ;
    }

    std::cerr << "WARN: only setting an outbound midi connection is not yet supported. " << std::endl ;
    return false ;
}

bool ApiHandler::handleModulationConnection(Engine* engine, ConnectionRequest request){
    if ( ! request.outboundID.has_value() || ! request.inboundID.has_value() ){
        std::cerr << "WARN: modulation connections must have valid IDs for both inbound and outbound objects." << std::endl ;
        return false ;
    }

    BaseModulator* modulator = engine->modulationController.getRaw(request.outboundID.value());

    if ( request.inboundIsModule.value() ){
        engine->moduleController.getRaw(request.inboundID.value())->setParameterModulation(request.inboundParameter.value(), modulator);
    } else {
        engine->modulationController.getRaw(request.inboundID.value())->setParameterModulation(request.inboundParameter.value(), modulator);
    }

    return true ;
}

// utility function to convert a json field to a variant
ParameterValue ApiHandler::json2Variant(const json& j) {
    if (j.is_boolean()) return j.get<bool>() ;
    
    if (j.is_number()) {
        if (j.is_number_integer()) {
            int val = j.get<int>();
            if (val >= 0 && val <= 255) {
                return static_cast<uint8_t>(val);
            }
            return val;
        }
        
        if (j.is_number_float()) {
            return j.get<double>();
        }
    }
    
    throw std::runtime_error("Unsupported JSON type");
}