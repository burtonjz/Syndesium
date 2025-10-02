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

#ifndef __MODULATION_CONTROLLER_HPP_
#define __MODULATION_CONTROLLER_HPP_

#include "core/ComponentController.hpp"
#include "modulation/BaseModulator.hpp"
#include "configs/ModulatorConfig.hpp"

#include "modulation/LinearFader.hpp"
#include "modulation/ADSREnvelope.hpp"

#include <cstddef>
#include <memory>
#include <utility>
#include <type_traits>
#include <unordered_map>

using ModulatorID = std::size_t ;
using ModulatorKey = std::pair<ModulatorType, int>;

#define HANDLE_CREATE_MODULATOR(Type) \
    case ModulatorType::Type: \
        return create<ModulatorType::Type>(name,  j.get<Type##Config>());

struct ModulatorKeyHash {
    std::size_t operator()(const ModulatorKey& key) const {
        return std::hash<int>()(static_cast<int>(key.first)) ^ (std::hash<int>()(key.second) << 1);
    }
};

class ModulationController: public ComponentController<
    BaseModulator, 
    ModulatorType, 
    ModulatorKey, 
    ModulatorKeyHash, 
    ModulatorTypeTraits
> {
public:
    ComponentId dispatchFromJson(ModulatorType type, const std::string& name, const json& j){
        switch (type){
            HANDLE_CREATE_MODULATOR(LinearFader)
            HANDLE_CREATE_MODULATOR(ADSREnvelope)
        default:
            throw std::invalid_argument("Unsupported ModulatorType");
        } 
    }

    void tick(float dt){
        for (auto it = components_.begin(); it != components_.end(); ++it){
            it->second->tick(); // modulate parameters owned by modulator
        }
    }
};

#endif // __MODULATION_CONTROLLER_HPP_