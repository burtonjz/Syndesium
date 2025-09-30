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

#ifndef __HPP_CONFIGS_OSCILLATOR_
#define __HPP_CONFIGS_OSCILLATOR_

#include "types/ModuleType.hpp"
#include "types/Waveform.hpp"

#include <nlohmann/json.hpp>
using json = nlohmann::json ;

// forward declare class
class Oscillator ;

// define default configuration
struct OscillatorConfig {
    Waveform waveform = Waveform::SINE ;
    double frequency = 440.0 ;
};

// Type Traits Specification
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(OscillatorConfig, waveform) // macro to serialize/deserialize json <-> structs

template <> struct ModuleTypeTraits<ModuleType::Oscillator>{ 
    using type = Oscillator ;
    using config = OscillatorConfig ;
};

#endif // __HPP_CONFIGS_OSCILLATOR_