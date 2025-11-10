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

#include "types/ParameterType.hpp"

ParameterType parameterFromString(std::string str) {
    static const std::unordered_map<std::string, ParameterType> str2Type = {
        #define X(NAME) {ParameterTraits<ParameterType::NAME>::name, ParameterType::NAME},
        PARAMETER_TYPE_LIST
        #undef X
    };
    
    auto it = str2Type.find(str);
    if (it != str2Type.end()) {
        return it->second;
    }
    throw std::runtime_error("Unknown parameter name: " + str);
}