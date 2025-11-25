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
#include "core/BaseModule.hpp"
#include "core/Engine.hpp"
#include "config/Config.hpp"
#include "configs/ComponentConfig.hpp"
#include "types/SocketType.hpp"

#include <iostream>
#include <netinet/in.h>
#include <sys/socket.h>
#include <fcntl.h>
#include <thread>
#include <optional>

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
}

void ApiHandler::start(){
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
            std::thread([clientSock](){
                ApiHandler::instance()->onClientConnection(clientSock);
            }).detach();  
        } else {
            // no pending connection; sleep briefly to avoid busy-wait
            std::this_thread::sleep_for(std::chrono::milliseconds(1000));
        }
    }

    // Close the server socket
    close(serverSock);
}

void ApiHandler::onClientConnection(int clientSock){
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
                handleClientMessage(clientSock, jsonStr);
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

void ApiHandler::sendApiResponse(int clientSock, json& response, const std::string& err){
    if ( err == "" ){
        response["status"] = "success" ;
    } else {
        response["status"] = "failed" ;
        response["error"] = err ;
    }
    std::string r = response.dump() + '\n' ;
    std::cout << "sending API response: " << r.c_str() << std::endl  ;
    send(clientSock, r.c_str(), r.size(), 0);
}


void ApiHandler::handleClientMessage(int clientSock, std::string jsonStr){
    json jResponse ;
    json jRequest ;

    std::string err ;

    try {
        jRequest = json::parse(jsonStr);

        std::string action = jRequest["action"] ;
        jResponse["action"] = action ;

        std::cout << "received request for action: " << action << std::endl ;

        if ( action == "get_audio_devices" ) {
            jResponse["data"] = engine_->getAvailableAudioDevices() ;
            sendApiResponse(clientSock,jResponse);
            return ;
        }

        if ( action == "get_midi_devices" ){
            jResponse["data"] = engine_->getAvailableMidiDevices() ;
            sendApiResponse(clientSock,jResponse);
            return ;
        }

        if ( action == "set_audio_device" ){
            int deviceId = jRequest["device_id"] ;
            engine_->setAudioDeviceId(deviceId);
            sendApiResponse(clientSock,jResponse);
            return ;
        } 
        
        if ( action == "set_midi_device" ){
            int deviceId = jRequest["device_id"] ;
            engine_->setMidiDeviceId(deviceId);
            sendApiResponse(clientSock,jResponse);
            return ;
        }

        if ( action == "set_state" ){
            jResponse["state"] = jRequest["state"] ;
            if ( jResponse["state"] == "run" ){
                engine_->run();
                sendApiResponse(clientSock,jResponse);
                return ;
            } 
            
            if ( jResponse["state"] == "stop" ){
                engine_->stop();
                sendApiResponse(clientSock,jResponse);
                return ;
            } 
            
            err = "Unrecognized engine state requested: " + jResponse["state"].get<std::string>() ;
            sendApiResponse(clientSock,jResponse, err);
            return ;
        }

        if ( action == "get_configuration"){
            jResponse["data"] = engine_->serialize();
            sendApiResponse(clientSock,jResponse);
            return ;
        }
        
        if ( action == "load_configuration" ){
            return ;
        }

        if ( action == "get_waveforms" ){
            jResponse["data"] = Waveform::getWaveforms() ;
            sendApiResponse(clientSock,jResponse);
            return ;
        }
        // COMPONENTS
        if ( action == "add_component"){
            jResponse["type"] = jRequest["type"] ;
            jResponse["name"] = jRequest["name"] ;

            ComponentType type = static_cast<ComponentType>(jRequest["type"]);
            ComponentId id = engine_->componentFactory.createFromJson(type, jResponse["name"], getDefaultConfig(jResponse["type"]));

            jResponse["component_id"] = id;
            sendApiResponse(clientSock,jResponse);
            return ;
        }

        if ( action == "set_component_parameter"){
            int componentId = jRequest["componentId"];
            ParameterType p = static_cast<ParameterType>(jRequest["parameter"]);

            jResponse["status"] = engine_->componentManager.setComponentParameter(componentId, p, jRequest["value"]);
            jResponse["componentId"] = jRequest["componentId"];
            jResponse["parameter"] = jRequest["parameter"];

            if ( jResponse["status"] ){
                sendApiResponse(clientSock,jResponse);
                return ;
            } else {
                sendApiResponse(clientSock,jResponse, "Error setting component parameter." );
                return ;
            }
        }

        if ( action == "create_connection" ){
            // save variables into response
            jResponse["connectionID"] = jRequest["connectionID"];
            jResponse["inbound"] = jRequest["inbound"] ;
            jResponse["outbound"] = jRequest["outbound"] ;

            ConnectionRequest request = parseConnectionRequest(jRequest);
            if ( routeConnectionRequest(request)){
                sendApiResponse(clientSock,jResponse);
                return ;
            } else {
                sendApiResponse(clientSock,jResponse, "failed to make requested connection");
                return ;
            }
        }

        if ( action == "remove_connection" ){
            jResponse["connectionID"] = jRequest["connectionID"];
            jResponse["inbound"] = jRequest["inbound"] ;
            jResponse["outbound"] = jRequest["outbound"] ; 

            ConnectionRequest request = parseConnectionRequest(jRequest);
            request.remove = true ;

            if ( routeConnectionRequest(request)){
                sendApiResponse(clientSock,jResponse);
                return ;
            } else {
                sendApiResponse(clientSock,jResponse, "failed to remove specified connection");
                return ;
            }
        }

        err = "Unknown action requested: " + action ;
        sendApiResponse(clientSock,jResponse, err);

    } catch (const std::exception& e){
        err = "JSON parse error: " + std::string(e.what()) ;
        sendApiResponse(clientSock,jResponse, err);
    }
}

bool ApiHandler::loadConfiguration(json request){

}

ConnectionRequest ApiHandler::parseConnectionRequest(json request){
    ConnectionRequest req ;
    // define API variables
    req.inboundSocket = static_cast<SocketType>(request["inbound"]["socketType"]);
    req.outboundSocket = static_cast<SocketType>(request["outbound"]["socketType"]);
    if ( request["inbound"].contains("id")) req.inboundID = request["inbound"]["id"];
    if ( request["outbound"].contains("id")) req.outboundID = request["outbound"]["id"];
    if ( request["inbound"].contains("parameter")) req.inboundParameter = static_cast<ParameterType>(request["inbound"]["parameter"]);

    return req ;
}

bool ApiHandler::routeConnectionRequest(ConnectionRequest request){
    if ( request.inboundSocket == SocketType::MidiInput && request.outboundSocket == SocketType::MidiOutput )
        return handleMidiConnection(request);
    
    if ( request.inboundSocket == SocketType::SignalInput && request.outboundSocket == SocketType::SignalOutput )
        return handleSignalConnection(request);

    if ( request.inboundSocket == SocketType::ModulationInput && request.outboundSocket == SocketType::ModulationOutput )
        return handleModulationConnection(request);

    std::cerr << "WARN: socket types are incompatible. No connection will be made" << std::endl ;
    return false ;
}

bool ApiHandler::handleSignalConnection(ConnectionRequest request){
    BaseModule* inbound = nullptr ;
    BaseModule* outbound = nullptr ;

    inbound = engine_->componentManager.getModule(request.inboundID.value_or(-1));
    outbound = engine_->componentManager.getModule(request.outboundID.value_or(-1));

    // if the source is an external endpoint
    if ( ! request.outboundID.has_value() ){
        std::cerr << "receiving audio from an input device is not yet supported." << std::endl ;
        return false ;
    }

    // if the destination is an external endpoint
    if ( ! request.inboundID.has_value() ){
        if ( request.remove ){
            engine_->signalController.unregisterSink(outbound);
            return true ;
        }
        engine_->signalController.registerSink(outbound);
        return true ;
    }
    
    if ( request.remove ){
        engine_->signalController.disconnect(outbound, inbound);
        return true ;
    }

    engine_->signalController.connect(outbound, inbound);
    return true ;
}

bool ApiHandler::handleMidiConnection(ConnectionRequest request){
    if ( ! request.outboundID.has_value() && ! request.inboundID.has_value() ){
        std::cerr << "WARN: midi connection was requested with an invalid object (no id supplied for inbound or outbound objects)." << std::endl ;
        return false ;
    }

    if ( request.outboundID.has_value() && request.inboundID.has_value() ){
        // this is a standard connection between a handler and a listener
        MidiEventHandler* handler = engine_->componentManager.getMidiHandler(request.outboundID.value());
        MidiEventListener* listener = engine_->componentManager.getMidiListener(request.inboundID.value());

        if ( !listener ){
            std::cerr << "WARN: No valid MidiEventListener found from connection request configuration" << std::endl ;
            return false ;
        }
        
        if ( !handler ){
            std::cerr << "WARN: No valid MidiEventHandler found from connection request configuration" << std::endl ;
            return false ;
        }

        if ( request.remove ){
            return engine_->removeMidiConnection(handler, listener);
        }

        return engine_->setMidiConnection(handler, listener);    
    }

    if ( ! request.outboundID.has_value() && request.inboundID.has_value() ){
        MidiEventHandler* inboundHandler = engine_->componentManager.getMidiHandler(request.inboundID.value());
        MidiEventListener* inboundListener = engine_->componentManager.getMidiListener(request.inboundID.value());

        // case 1: inbound is a handler, so we need to register this handler against the MidiState (receive raw midi events)
        if ( inboundHandler ){
            if ( request.remove ){
                engine_->unregisterBaseMidiHandler(inboundHandler);
                return true ;
            }
            engine_->registerBaseMidiHandler(inboundHandler);
            return true ;
        }

        // case 2: inbound is only a listener (so we register to the default handler)
        if ( inboundListener ){
            if ( request.remove ){
                engine_->removeMidiConnection(engine_->getDefaultMidiHandler(), inboundListener);
                return true ;
            }
            engine_->setMidiConnection(engine_->getDefaultMidiHandler(), inboundListener);
            return true ;
        }

        std::cerr << "WARN: midi connection was requested with an invalid object." << std::endl ;
        return false ;
    }

    std::cerr << "WARN: only setting an outbound midi connection is not yet supported. " << std::endl ;
    return false ;
}

bool ApiHandler::handleModulationConnection(ConnectionRequest request){
    if ( ! request.outboundID.has_value() || ! request.inboundID.has_value() ){
        std::cerr << "WARN: modulation connections must have valid IDs for both inbound and outbound objects." << std::endl ;
        return false ;
    }

    BaseModulator* modulator = engine_->componentManager.getModulator(request.outboundID.value());
    BaseComponent* component = engine_->componentManager.getRaw(request.inboundID.value());
    std::cout << "modulator with id = " << request.outboundID.value() << " is at address " << modulator << std::endl ;

    if (!modulator ){
        std::cerr << "valid modulator not found.\n";
        return false ;
    }

    if (!component){
        std::cerr << "valid component not found.\n";
        return false ;
    }

    if ( request.remove ){
        component->removeParameterModulation(request.inboundParameter.value());
        // stateful modulators need to be in the signal processing graph
        if ( dynamic_cast<BaseModule*>(modulator) ){
            engine_->signalController.updateProcessingGraph();
        }
        return true ;
    }
    
    component->setParameterModulation(request.inboundParameter.value(), modulator);
    // stateful modulators need to be in the signal processing graph
    if ( dynamic_cast<BaseModule*>(modulator) ){
        engine_->signalController.updateProcessingGraph();
    }
    return true ;
}
