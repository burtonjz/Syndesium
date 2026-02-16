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

#ifndef I_SOCKET_LOOKUP_HPP_
#define I_SOCKET_LOOKUP_HPP_

#include "types/SocketType.hpp"
#include "types/ParameterType.hpp"

#include <optional>
#include <QPointF>

// forward declarations
class SocketWidget ;

class ISocketLookup {
public:
    virtual ~ISocketLookup() = default ;
    
    /**
     * @brief find a socket widget
     * 
     * @param componentId component the socket belongs to
     * @param type socket type
     * @param idx if audio socket, specify audio socket index
     * @param param if modulation socket, specify modulation parameter
     * @return SocketWidget* 
     */
    virtual SocketWidget* findSocket(
        SocketType type,
        std::optional<int> componentId = std::nullopt,
        std::optional<size_t> idx = std::nullopt, 
        std::optional<ParameterType> param = std::nullopt        
    ) = 0 ;

    /**
     * @brief find a socket widget at the specified location
     * 
     * @param scenePos 
     * @return SocketWidget* 
     */
    virtual SocketWidget* findSocketAt(const QPointF& scenePos) = 0 ;

};

#endif // I_SOCKET_LOOKUP_HPP_
