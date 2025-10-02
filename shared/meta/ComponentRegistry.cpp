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
#include "types/ModuleType.hpp"
#include "types/ParameterType.hpp"
#include <stdexcept>

const std::unordered_map<ComponentType, ComponentDescriptor>& ComponentRegistry::getAllComponentDescriptors(){
    static const std::unordered_map<ComponentType, ComponentDescriptor> registry = {
        {
            ComponentType(ModuleType::Oscillator),
            {
                "Oscillator",
                ComponentType(ModuleType::Oscillator),
                {ParameterType::AMPLITUDE, ParameterType::FREQUENCY}, // modulatable params
                {ParameterType::WAVEFORM, ParameterType::AMPLITUDE, ParameterType::FREQUENCY}, //control params
                0, // audio inputs
                1, // audio outputs
                0, // midi inputs
                0, // midi outputs
                false  // polyphonic
            }
        },
        {
            ComponentType(ModuleType::PolyOscillator), 
            {
                "Polyphonic Oscillator",
                ComponentType(ModuleType::PolyOscillator),
                {ParameterType::AMPLITUDE, ParameterType::FREQUENCY, ParameterType::PHASE}, // modulatable params
                {ParameterType::WAVEFORM}, //control params
                0, // audio inputs
                1, // audio outputs
                1, // midi inputs
                0, // midi outputs
                true  // polyphonic
            }
        },
        {
            ComponentType(ModulatorType::LinearFader),
            {
                "Linear Fader",
                ComponentType(ModulatorType::LinearFader),
                {ParameterType::ATTACK, ParameterType::RELEASE},
                {ParameterType::ATTACK, ParameterType::RELEASE},
                0,
                0,
                1,
                1,
                false
            }
        },
        {
            ComponentType(ModulatorType::ADSREnvelope),
            {
                "ADSR Envelope",
                ComponentType(ModulatorType::ADSREnvelope),
                {ParameterType::ATTACK, ParameterType::DECAY, ParameterType::SUSTAIN, ParameterType::RELEASE},
                {ParameterType::ATTACK, ParameterType::DECAY, ParameterType::SUSTAIN, ParameterType::RELEASE},
                0,
                0,
                1,
                1,
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

const ComponentDescriptor& ComponentRegistry::getComponentDescriptor(ModuleType type){
    return getComponentDescriptor(ComponentType(type));
}

const ComponentDescriptor& ComponentRegistry::getComponentDescriptor(ModulatorType type){
    return getComponentDescriptor(ComponentType(type));
}