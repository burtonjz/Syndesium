#include "api/ApiHandler.hpp"
#include "Engine.hpp"
#include "config/Config.hpp"
#include "types/SocketType.hpp"

#include <iostream>
#include <netinet/in.h>
#include <sys/socket.h>
#include <fcntl.h>
#include <thread>

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

        // MODULES
        if ( action == "add_module"){
            ModuleType type = static_cast<ModuleType>(jRequest["type"]) ;
            ModuleID id = engine->moduleController.dispatchFromJson(
                type,
                jRequest["name"], 
                Module::getDefaultConfig(type));
            jResponse["module_id"] = id ;
            jResponse["type"] = jRequest["type"];
            sendSuccess();
            return ;
        }

        if ( action == "get_waveforms" ){
            jResponse["data"] = Waveform::getWaveforms() ;
            sendSuccess();
            return ;
        }

        if ( action == "get_module_parameter" ){
            int moduleID = jRequest["id"];
            ParameterType p = static_cast<ParameterType>(jRequest["parameter"]);
            Module::BaseModule* module = engine->moduleController.getRaw(moduleID);
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
            Modulator* modulator = engine->modulationController.getRaw(modulatorID);
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
            // define API variables
            SocketType inputSocket = static_cast<SocketType>(jRequest["input"]["socket"]);
            SocketType outputSocket = static_cast<SocketType>(jRequest["output"]["socket"]);
            std::optional<int> inputID, outputID ; 
            if ( jRequest["input"].contains("id") ) inputID = jRequest["input"]["id"] ;
            if ( jRequest["output"].contains("id") ) outputID = jRequest["output"]["id"] ;
            
            // Case 1: Midi -> Midi
            if ( outputSocket == SocketType::MidiOutput && inputSocket == SocketType::MidiInput){
                // output midi should be a modulator (or nullptr for default handler )
                MidiEventHandler*  handler ;
                if ( outputID.has_value() ){
                    handler = dynamic_cast<MidiEventHandler*> (engine->modulationController.getRaw(outputID.value()));
                } else {
                    handler = nullptr ; 
                }

                MidiEventListener* listener ;
                if ( inputID.has_value() ){
                    listener = dynamic_cast<MidiEventListener*>(engine->moduleController.getRaw(inputID.value()));
                } else {
                    // outbound hardware MIDI not yet implemented
                    sendError("sending midi to an external device is not yet supported.");
                    return ;
                }
                
                if ( ! engine->setMidiConnection(handler, listener) ){
                    sendError("midi connection not set successfully.");
                    return ;
                }
                
                sendSuccess();
                return ;
            }

            // Case 2: Signal -> Signal
            if ( outputSocket == SocketType::SignalOutput && inputSocket == SocketType::SignalInput ){
                auto inputModule = engine->moduleController.getRaw(inputID.value_or(engine->moduleController.getAudioInputID()));
                auto outputModule = engine->moduleController.getRaw(outputID.value_or(engine->moduleController.getAudioOutputID()));
                
                // if the source is hardware
                if ( !outputID.has_value() ){
                    sendError("receiving audio from an input device is not yet supported.");
                    return ;
                }

                // if the destination is hardware
                if ( !inputID.has_value() ){
                    engine->moduleController.registerSink(outputModule);
                    jResponse["status"] = "success" ;
                    sendApiResponse(clientSock, jResponse);
                    return ;
                }
                
                engine->moduleController.connect(outputModule, inputModule);
                jResponse["status"] = "success" ;
                sendApiResponse(clientSock, jResponse);
                return ;
            }

            // Case 3: Signal -> Modulator
            sendSuccess();
        }

        err = "Unknown action requested: " + action ;
        sendError(err);

    } catch (const std::exception& e){
        err = "JSON parse error: " + std::string(e.what()) ;
        sendError(err);
    }
}