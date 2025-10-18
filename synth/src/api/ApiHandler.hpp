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
public:
    ApiHandler() = delete ;

    static void start(Engine* engine);
    static void onClientConnection(Engine* engine, int clientSock);
    static void handleClientMessage(Engine* engine, int clientSock, std::string jsonStr);
    static void sendApiResponse(int clientSock, json response);

private:
    // cable connection functions
    static ConnectionRequest parseConnectionRequest(json request);
    static bool routeConnectionRequest(Engine* engine, ConnectionRequest request);
    static bool handleSignalConnection(Engine* engine, ConnectionRequest request);
    static bool handleMidiConnection(Engine* engine, ConnectionRequest request);
    static bool handleModulationConnection(Engine* engine, ConnectionRequest request);

    // utility functions
    // static ParameterValue json2Variant(const json& j);
};


#endif // __API_HANDLER_HPP_
