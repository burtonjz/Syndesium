#include "api/ApiHandler.hpp"
#include "Engine.hpp"
#include "config/Config.hpp"

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
    try {
        jRequest = json::parse(jsonStr);

        std::string action = jRequest["action"] ;
        jResponse["action"] = action ;

        std::cout << "received request for action: " << action << std::endl ;

        if ( action == "get_audio_devices" ) {
            jResponse["data"] = engine->getAvailableAudioDevices() ;
            sendApiResponse(clientSock, jResponse);
            return ;
        }
        if ( action == "get_midi_devices" ){
            jResponse["data"] = engine->getAvailableMidiDevices() ;
            sendApiResponse(clientSock, jResponse);
            return ;
        }
        if ( action == "set_audio_device" ){
            int deviceId = jRequest["device_id"] ;
            engine->setAudioDeviceId(deviceId);
            jResponse["status"] = "success" ; 
            sendApiResponse(clientSock, jResponse);
            return ;
        } 
        
        if ( action == "set_midi_device" ){
            int deviceId = jRequest["device_id"] ;
            engine->setMidiDeviceId(deviceId);
            jResponse["status"] = "success" ; 
            sendApiResponse(clientSock, jResponse);
            return ;
        }
        if ( action == "set_state" ){
            jResponse["state"] = jRequest["state"] ;
            if ( jResponse["state"] == "run" ){
                std::cout << "Running engine. " << std::endl ;
                engine->run();
                jResponse["status"] = "success" ;
                sendApiResponse(clientSock, jResponse);
            } else if ( jResponse["state"] == "stop" ){
                std::cout << "Stopping engine. " << std::endl ;
                engine->stop();
                jResponse["status"] = "success" ;
                sendApiResponse(clientSock, jResponse);
            } else {
                std::cout << "Unrecognized engine state requested: " << jResponse["state"] << std::endl ;
            }
            return ;
        }

        // MODULES
        if ( action == "get_waveforms" ){
            jResponse["data"] = Waveform::getWaveforms() ;
            sendApiResponse(clientSock, jResponse);
            return ;
        }

        if ( action == "get_module_parameter" ){
            int moduleID = jRequest["id"];
            ParameterType p = static_cast<ParameterType>(jRequest["parameter"]);
            Module::BaseModule* module = engine->moduleController.getRaw(moduleID);
            if  (!module){
                std::cerr << "unable to find modulator of with ID" << moduleID <<  std::endl ;
            }
            auto params = module->getParameters()->getBaseMap() ;
            auto it = params.find(p);
            if (it == params.end()){
                std::cerr << "ParameterType " << static_cast<int>(p) << "is not present in map for ModuleID " << moduleID << std::endl ; 
                return ;
            }
            jRequest["data"] = ParameterMap::dispatchToJson(module->getParameters()->getValueDispatch(p));
        }
        
        // MODULATION
        // TODO: write "set_modulator_parameter" )
        if ( action == "get_modulator_parameter" ){
            int modulatorID = jRequest["id"];
            ParameterType p = static_cast<ParameterType>(jRequest["parameter"]);
            Modulator* modulator = engine->modulationController.getRaw(modulatorID);
            if (!modulator){
                std::cerr << "unable to find modulator of with ID" << modulatorID <<  std::endl ;
                return ;
            }
            auto params = modulator->getParameters()->getBaseMap() ;
            auto it = params.find(p);
            if (it == params.end()){
                std::cerr << "ParameterType " << static_cast<int>(p) << "is not present in map for ModulatorID " << modulatorID << std::endl ; 
                return ;
            }
            jRequest["data"] = ParameterMap::dispatchToJson(modulator->getParameters()->getValueDispatch(p));
             
            sendApiResponse(clientSock, jResponse);
            return ;
        }

        std::cerr << "Unknown action requested: " << action << std::endl ;
        jResponse["status"] = "failed";
        sendApiResponse(clientSock, jResponse);
        
    } catch (const std::exception& e){
        std::cerr << "JSON parse error: " << e.what() << std::endl ;
    }
}