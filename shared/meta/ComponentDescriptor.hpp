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

#include <stdexcept>
#include <string>
#include <set>
#include <cstdint>
#include <nlohmann/json.hpp>

#include "types/ParameterType.hpp"
#include "types/ModuleType.hpp"
#include "types/ModulatorType.hpp"


using json = nlohmann::json ;

struct ComponentType {
    uint32_t moduleType ;
    uint32_t modulatorType ;

    static constexpr uint32_t NONE = UINT32_MAX ;

    // Constructors
    ComponentType(ModuleType type):
        moduleType(static_cast<uint32_t>(type)),
        modulatorType(NONE){}

    ComponentType(ModulatorType type): 
        moduleType(NONE),
        modulatorType(static_cast<uint32_t>(type)){}

    ComponentType(ModuleType type, ModulatorType modType): 
        moduleType(static_cast<uint32_t>(type)), 
        modulatorType(static_cast<uint32_t>(modType)){}

    bool isModule() const { return moduleType != NONE ;}
    bool isModulator() const { return modulatorType != NONE ;}
    bool isHybrid() const { return isModule() && isModulator() ; }

    ModuleType getModuleType() const {
        if (!isModule()) throw std::runtime_error("Not a module type");
        return static_cast<ModuleType>(moduleType);
    }

    ModulatorType getModulatorType() const {
        if (!isModulator()) throw std::runtime_error("Not a module type");
        return static_cast<ModulatorType>(modulatorType);
    }

    bool operator==(const ComponentType& other) const {
        return moduleType == other.moduleType && modulatorType == other.modulatorType ;
    }
};

// hash specialization
namespace std {
    template<>
    struct hash<ComponentType> {
        std::size_t operator()(const ComponentType& ct) const {
            return std::hash<uint64_t>{}(
                (static_cast<uint64_t>(ct.moduleType) << 32 ) | ct.modulatorType
            );
        }
    };
}

struct ComponentDescriptor {
    std::string name ;
    ComponentType type ;
    std::set<ParameterType> modulatableParameters ;
    std::set<ParameterType> controllableParameters ;

    size_t numAudioInputs  ;
    size_t numAudioOutputs ;
    size_t numMidiInputs ;
    size_t numMidiOutputs ;

    bool isPolyphonic = false ;

};

#endif // __SHARED_COMPONENT_DESCRIPTOR_HPP_