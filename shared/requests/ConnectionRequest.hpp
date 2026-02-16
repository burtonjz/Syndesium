/*
 * Copyright (C) 2026 Jared Burton
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

#ifndef CONNECTION_REQUEST_HPP_
#define CONNECTION_REQUEST_HPP_

#include "types/SocketType.hpp"
#include "types/ParameterType.hpp"
#include <optional>
#include <nlohmann/json.hpp>
#include <stdexcept>

using json = nlohmann::json ;

struct ConnectionRequest {
    SocketType inboundSocket ;
    SocketType outboundSocket ;
    std::optional<int> inboundID ;
    std::optional<size_t> inboundIdx ; // audio only
    std::optional<int> outboundID ;
    std::optional<size_t> outboundIdx ; // audio only
    std::optional<ParameterType> inboundParameter ;
    bool remove = false ;

    bool operator==(const ConnectionRequest& other) const {
        return inboundSocket == other.inboundSocket &&
               outboundSocket == other.outboundSocket &&
               inboundID == other.inboundID &&
               inboundIdx == other.inboundIdx &&
               outboundID == other.outboundID &&
               outboundIdx == other.outboundIdx &&
               inboundParameter == other.inboundParameter ;
    }

    bool valid() const {
        bool t = true ;
        switch(inboundSocket){
        case SocketType::SignalInbound:
            t = t && outboundSocket == SocketType::SignalOutbound ;
            t = t && inboundID.has_value() == inboundIdx.has_value();
            t = t && outboundID.has_value() == outboundIdx.has_value();
            break ;
        case SocketType::MidiInbound:
            t = t && outboundSocket == SocketType::MidiOutbound ;
            break ;
        case SocketType::ModulationInbound:
            t = t && outboundSocket == SocketType::ModulationOutbound ;
            t = t && inboundParameter.has_value() ;
            break ;
        default:
            t = false ;
            break ;
        }

        return t ;
    }
};

inline void to_json(json& j, const ConnectionRequest& req){
    if ( req.remove ){
        j["action"] = "remove_connection" ;
    } else {
        j["action"] = "create_connection" ;
    }

    j["inbound"];
    j["outbound"];
    j["inbound"]["socketType"] = req.inboundSocket ;
    j["outbound"]["socketType"] = req.outboundSocket ;
    if ( req.inboundID.has_value() ) j["inbound"]["componentId"] = req.inboundID.value();
    if ( req.inboundIdx.has_value() ) j["inbound"]["index"] = req.inboundIdx.value();
    if ( req.outboundID.has_value() ) j["outbound"]["componentId"] = req.outboundID.value();
    if ( req.outboundIdx.has_value() ) j["outbound"]["index"] = req.outboundIdx.value();
    if ( req.inboundParameter.has_value() ) j["inbound"]["parameter"] = req.inboundParameter.value();
}

inline void from_json(const json& j, ConnectionRequest& req){
    req.inboundSocket = static_cast<SocketType>(j["inbound"]["socketType"]);
    req.outboundSocket = static_cast<SocketType>(j["outbound"]["socketType"]);
    if ( j["inbound"].contains("componentId")) req.inboundID = j["inbound"]["componentId"];
    if ( j["inbound"].contains("index")) req.inboundIdx = j["inbound"]["index"];
    if ( j["outbound"].contains("componentId")) req.outboundID = j["outbound"]["componentId"];
    if ( j["outbound"].contains("index")) req.outboundIdx = j["outbound"]["index"];
    if ( j["inbound"].contains("parameter")) req.inboundParameter = static_cast<ParameterType>(j["inbound"]["parameter"]);
    if ( j["action"] == "create_connection" ){
        req.remove = false ;
    } else if ( j["action"] == "remove_connection" ){
        req.remove = true ;
    } else {
        throw std::runtime_error("invalid action specified for connection request.");
    }
}

#endif // CONNECTION_REQUEST_HPP_