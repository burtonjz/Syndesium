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

#include "meta/ComponentRegistry.hpp"
#include "meta/ComponentDescriptor.hpp"
#include "types/ComponentType.hpp"
#include "types/ParameterType.hpp"
#include <stdexcept>

const std::unordered_map<ComponentType, ComponentDescriptor>& ComponentRegistry::getAllComponentDescriptors(){
    static const std::unordered_map<ComponentType, ComponentDescriptor> registry = {
        {
            ComponentType::Oscillator,
            {
                "Oscillator",
                ComponentType::Oscillator,
                {ParameterType::AMPLITUDE, ParameterType::FREQUENCY}, // modulatable params
                {ParameterType::WAVEFORM, ParameterType::AMPLITUDE, ParameterType::FREQUENCY}, //control params
                0, // audio inputs
                1, // audio outputs
                0, // midi inputs
                0, // midi outputs
                false, // modulator
                false  // polyphonic
            }
        },
        {
            ComponentType::PolyOscillator, 
            {
                "Polyphonic Oscillator",
                ComponentType::PolyOscillator,
                {ParameterType::AMPLITUDE, ParameterType::FREQUENCY, ParameterType::PHASE}, // modulatable params
                {ParameterType::WAVEFORM}, //control params
                0, // audio inputs
                1, // audio outputs
                1, // midi inputs
                0, // midi outputs
                false, // modulator
                true  // polyphonic
            }
        },
        {
            ComponentType::LinearFader,
            {
                "Linear Fader",
                ComponentType::LinearFader,
                {ParameterType::ATTACK, ParameterType::RELEASE},
                {ParameterType::ATTACK, ParameterType::RELEASE},
                0,
                0,
                1,
                1,
                true,
                false
            }
        },
        {
            ComponentType::ADSREnvelope,
            {
                "ADSR Envelope",
                ComponentType::ADSREnvelope,
                {ParameterType::ATTACK, ParameterType::DECAY, ParameterType::SUSTAIN, ParameterType::RELEASE},
                {ParameterType::ATTACK, ParameterType::DECAY, ParameterType::SUSTAIN, ParameterType::RELEASE},
                0,
                0,
                1,
                1,
                true,
                false
            }
        },
        {
            ComponentType::MidiFilter,
            {
                "Midi Filter",
                ComponentType::MidiFilter,
                {},
                {ParameterType::MIN_VALUE, ParameterType::MAX_VALUE},
                0,
                0,
                1,
                1,
                false,
                false
            }
        }
    };

    return registry ;
}

const ComponentDescriptor& ComponentRegistry::getComponentDescriptor(ComponentType type){
    const auto& registry = ComponentRegistry::getAllComponentDescriptors();
    auto  it =  registry.find(type);
    if (it != registry.end()){
        return  it->second ;
    }
    throw std::runtime_error("Unknown ComponentType");
}