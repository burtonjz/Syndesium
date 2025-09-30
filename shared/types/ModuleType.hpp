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

#ifndef __MODULE_TYPE_HPP_
#define __MODULE_TYPE_HPP_

/**
 * @brief enumeration of modulation source classes
 * 
*/
enum class ModuleType {
    Oscillator,
    PolyOscillator,
    N_MODULES
};

constexpr int N_MODULE_TYPES = static_cast<int>(ModuleType::N_MODULES) ;

// Define a converter from the enum to the actual type. 
// must be defined in synth/configs/
template <ModuleType Type> struct ModuleTypeTraits ;

// helper aliases
template<ModuleType Type>
using ModuleType_t = typename ModuleTypeTraits<Type>::type ;

template<ModuleType Type>
using ModuleConfig_t = typename ModuleTypeTraits<Type>::config ;

#endif // __MODULE_TYPE_HPP_