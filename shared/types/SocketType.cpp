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

#include "types/SocketType.hpp"
#include <stdexcept>
#include <algorithm>
#include <string>

const std::string socketType2String(SocketType s){
    return std::string(socketStrings[static_cast<int>(s)]);
}

SocketType socketTypeFromString(std::string str) {
    auto it = std::find(socketStrings.begin(), socketStrings.end(), str);
    if (it != socketStrings.end()) {
        return static_cast<SocketType>(std::distance(socketStrings.begin(), it));
    }
    // Handle not found case - throw exception or return sentinel value
    throw std::invalid_argument("Unknown parameter string: " + str);
}