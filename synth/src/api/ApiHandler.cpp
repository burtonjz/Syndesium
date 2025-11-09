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
                engine->run();
                sendSuccess();
                return ;
            } 
            
            if ( jResponse["state"] == "stop" ){
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
            jResponse["type"] = jRequest["type"] ;
            jResponse["name"] = jRequest["name"] ;

            ComponentType type = static_cast<ComponentType>(jRequest["type"]);
            ComponentId id = engine->componentFactory.createFromJson(type, jResponse["name"], getDefaultConfig(jResponse["type"]));

            jResponse["component_id"] = id;
            sendSuccess();
            return ;
        }

        if ( action == "set_component_parameter"){
            int componentId = jRequest["componentId"];
            ParameterType p = static_cast<ParameterType>(jRequest["parameter"]);

            jResponse["status"] = engine->componentManager.setComponentParameter(componentId, p, jRequest["value"]);
            jResponse["componentId"] = jRequest["componentId"];
            jResponse["parameter"] = jRequest["parameter"];

            if ( jResponse["status"] ){
                sendSuccess();
                return ;
            } else {
                sendError("Error setting component parameter.");
                return ;
            }
        }

        if ( action == "create_connection" ){
            // save variables into response
            jResponse["connectionID"] = jRequest["connectionID"];
            jResponse["inbound"] = jRequest["inbound"] ;
            jResponse["outbound"] = jRequest["outbound"] ;

            ConnectionRequest request = parseConnectionRequest(jRequest);
            if ( routeConnectionRequest(engine, request)){
                sendSuccess();
                return ;
            } else {
                sendError("failed to make requested connection");
                return ;
            }
        }

        if ( action == "remove_connection" ){
            jResponse["connectionID"] = jRequest["connectionID"];
            jResponse["inbound"] = jRequest["inbound"] ;
            jResponse["outbound"] = jRequest["outbound"] ; 

            ConnectionRequest request = parseConnectionRequest(jRequest);
            request.remove = true ;

            if ( routeConnectionRequest(engine, request)){
                sendSuccess();
                return ;
            } else {
                sendError("failed to remove specified connection");
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
    req.inboundSocket = static_cast<SocketType>(request["inbound"]["socket"]);
    req.outboundSocket = static_cast<SocketType>(request["outbound"]["socket"]);
    if ( request["inbound"].contains("id")) req.inboundID = request["inbound"]["id"];
    if ( request["outbound"].contains("id")) req.outboundID = request["outbound"]["id"];
    if ( request["inbound"].contains("parameter")) req.inboundParameter = static_cast<ParameterType>(request["inbound"]["parameter"]);

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

    inbound = engine->componentManager.getModule(request.inboundID.value_or(-1));
    outbound = engine->componentManager.getModule(request.outboundID.value_or(-1));

    // if the source is an external endpoint
    if ( ! request.outboundID.has_value() ){
        std::cerr << "receiving audio from an input device is not yet supported." << std::endl ;
        return false ;
    }

    // if the destination is an external endpoint
    if ( ! request.inboundID.has_value() ){
        if ( request.remove ){
            engine->signalController.unregisterSink(outbound);
            return true ;
        }
        engine->signalController.registerSink(outbound);
        return true ;
    }
    
    if ( request.remove ){
        engine->signalController.disconnect(outbound, inbound);
        return true ;
    }

    engine->signalController.connect(outbound, inbound);
    return true ;
}

bool ApiHandler::handleMidiConnection(Engine* engine, ConnectionRequest request){
    if ( ! request.outboundID.has_value() && ! request.inboundID.has_value() ){
        std::cerr << "WARN: midi connection was requested with an invalid object (no id supplied for inbound or outbound objects)." << std::endl ;
        return false ;
    }

    if ( request.outboundID.has_value() && request.inboundID.has_value() ){
        // this is a standard connection between a handler and a listener
        MidiEventHandler* handler = engine->componentManager.getMidiHandler(request.outboundID.value());
        MidiEventListener* listener = engine->componentManager.getMidiListener(request.inboundID.value());

        if ( !listener ){
            std::cerr << "WARN: No valid MidiEventListener found from connection request configuration" << std::endl ;
            return false ;
        }
        
        if ( !handler ){
            std::cerr << "WARN: No valid MidiEventHandler found from connection request configuration" << std::endl ;
            return false ;
        }

        if ( request.remove ){
            return engine->removeMidiConnection(handler, listener);
        }

        return engine->setMidiConnection(handler, listener);    
    }

    if ( ! request.outboundID.has_value() && request.inboundID.has_value() ){
        MidiEventHandler* inboundHandler = engine->componentManager.getMidiHandler(request.inboundID.value());
        MidiEventListener* inboundListener = engine->componentManager.getMidiListener(request.inboundID.value());

        // case 1: inbound is a handler, so we need to register this handler against the MidiState (receive raw midi events)
        if ( inboundHandler ){
            if ( request.remove ){
                engine->unregisterBaseMidiHandler(inboundHandler);
                return true ;
            }
            engine->registerBaseMidiHandler(inboundHandler);
            return true ;
        }

        // case 2: inbound is only a listener (so we register to the default handler)
        if ( inboundListener ){
            if ( request.remove ){
                engine->removeMidiConnection(engine->getDefaultMidiHandler(), inboundListener);
                return true ;
            }
            engine->setMidiConnection(engine->getDefaultMidiHandler(), inboundListener);
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

    BaseModulator* modulator = engine->componentManager.getModulator(request.outboundID.value());
    BaseComponent* component = engine->componentManager.getRaw(request.inboundID.value());
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
            engine->signalController.updateProcessingGraph();
        }
        return true ;
    }
    
    component->setParameterModulation(request.inboundParameter.value(), modulator);
    // stateful modulators need to be in the signal processing graph
    if ( dynamic_cast<BaseModule*>(modulator) ){
        engine->signalController.updateProcessingGraph();
    }
    return true ;
}
