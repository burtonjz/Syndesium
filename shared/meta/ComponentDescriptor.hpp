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

#ifndef __SHARED_COMPONENT_DESCRIPTOR_HPP_
#define __SHARED_COMPONENT_DESCRIPTOR_HPP_

#include <string>
#include <vector>
#include <nlohmann/json.hpp>

#include "types/ParameterType.hpp"
#include "types/ComponentType.hpp"

struct ComponentDescriptor {
    std::string name ;
    ComponentType type ;
    std::vector<ParameterType> modulatableParameters ;
    std::vector<ParameterType> controllableParameters ;

    size_t numAudioInputs  ;
    size_t numAudioOutputs ;
    size_t numMidiInputs ;
    size_t numMidiOutputs ;

    bool canModulate ;
    bool sequenceable = false ;

    bool isModule() const { return numAudioOutputs > 0 ; }
    bool isModulator() const { return canModulate ; }
    bool isMidiHandler() const { return numMidiOutputs > 0 ; }
    bool isMidiListener() const { return numMidiInputs > 0 ; }

};

#endif // __SHARED_COMPONENT_DESCRIPTOR_HPP_