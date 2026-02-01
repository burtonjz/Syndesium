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

#ifndef MULTIPLY_CONFIG_HPP_
#define MULTIPLY_CONFIG_HPP_

#include "types/ComponentType.hpp"
#include <nlohmann/json.hpp>

using json = nlohmann::json ;

// forward declare class
class Multiply ;

// define default configuration
struct MultiplyConfig {
    double scalar = 1.0 ;
};

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(MultiplyConfig, scalar) // macro to serialize/deserialize json <-> structs

template <> struct ComponentTypeTraits<ComponentType::Multiply>{ 
    using type = Multiply ;
    using config = MultiplyConfig ;
};

#endif // MULTIPLY_CONFIG_HPP_