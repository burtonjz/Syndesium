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

#ifndef __MODULATOR_TYPE_HPP_
#define __MODULATOR_TYPE_HPP_

/**
 * @brief enumeration of modulation source classes
 * 
*/
enum class ModulatorType {
    LinearFader,
    ADSREnvelope,
    Oscillator,
    MidiControl,
    N_MODULATORS
};

constexpr int N_MODULATOR_TYPES = static_cast<int>(ModulatorType::N_MODULATORS) ;

/* 
every ParameterType will store a default modulation strategy
based on the type of variable it is. This is overridable at the
Parameter level. 
*/ 
enum class ModulationStrategy {
    ADDITIVE,
    MULTIPLICATIVE, 
    EXPONENTIAL,
    LOGARITHMIC,
    REPLACE,
    NONE
};

// Define a converter from the enum to the actual type. 
// must be defined in every Modulator Header
template <ModulatorType Type> struct ModulatorTypeTraits ;

// helper aliases
template<ModulatorType Type>
using ModulatorType_t = typename ModulatorTypeTraits<Type>::type ;

template<ModulatorType Type>
using ModulatorConfig_t = typename ModulatorTypeTraits<Type>::config ;

#endif // __MODULATOR_TYPE_HPP_