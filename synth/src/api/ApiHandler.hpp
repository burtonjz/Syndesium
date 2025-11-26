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

#ifndef __API_HANDLER_HPP_
#define __API_HANDLER_HPP_

#include <nlohmann/json.hpp>
#include <functional>

#include "types/SocketType.hpp"
#include "types/ParameterType.hpp"
#include "params/ParameterMap.hpp"


using json = nlohmann::json ;

// forward declarations
class Engine ;

// useful structs for API JSON parsing
struct ConnectionRequest {
    SocketType inboundSocket ;
    SocketType outboundSocket ;
    std::optional<int> inboundID ;
    std::optional<int> outboundID ;
    std::optional<bool> inboundIsModule ;
    std::optional<bool> outboundIsModule ;
    std::optional<ParameterType> inboundParameter ;
    bool remove = false ;
};

class ApiHandler {
private:
    using HandlerFunc = std::function<json(int sock, const json& request)>;
    Engine* engine_ ;
    std::unordered_map<std::string, HandlerFunc> handlers_ ;

    ApiHandler();

public:
    static ApiHandler* instance() ;
    ApiHandler(const ApiHandler&) = delete ;
    ApiHandler& operator=(const ApiHandler&) = delete ;
    ApiHandler(ApiHandler&&) = delete ;
    ApiHandler& operator=(ApiHandler&&) = delete ;

    void initialize(Engine* engine);

    void start();
    void onClientConnection(int clientSock);
    void handleClientMessage(int clientSock, std::string jsonStr);
    json sendApiResponse(int clientSock, json& response, const std::string& err = "");

private:
    // handler functions
    json getAudioDevices(int sock, const json& request);
    json getMidiDevices(int sock, const json& request);
    json setAudioDevice(int sock, const json& request);
    json setMidiDevice(int sock, const json& request);
    json setState(int sock, const json& request);
    json getConfiguration(int sock, const json& request);
    json loadConfiguration(int sock, const json& request);
    json getWaveforms(int sock, const json& request);
    json addComponent(int sock, const json& request);
    json removeComponent(int sock, const json& request);
    json getComponentParameter(int sock, const json& request);
    json setComponentParameter(int sock, const json& request);
    json createConnection(int sock, const json& request);
    json removeConnection(int sock, const json& request);

    // load functions
    bool loadCreateComponent(int sock, const json& components, std::unordered_map<int,int>& idMap);
    bool loadConnectComponent(int sock, const json& components, const std::unordered_map<int,int>& idMap);

    // cable connection functions
    ConnectionRequest parseConnectionRequest(json request);
    bool routeConnectionRequest(ConnectionRequest request);
    bool handleSignalConnection(ConnectionRequest request);
    bool handleMidiConnection(ConnectionRequest request);
    bool handleModulationConnection(ConnectionRequest request);

};


#endif // __API_HANDLER_HPP_
