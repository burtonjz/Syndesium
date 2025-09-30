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

#ifndef __HPP_CONFIGS_ALL_MODULES
#define __HPP_CONFIGS_ALL_MODULES

#include "types/ModuleType.hpp"
#include "configs/OscillatorConfig.hpp"
#include "configs/PolyOscillatorConfig.hpp"

#include <nlohmann/json.hpp>
using json = nlohmann::json ;


#define HANDLE_DEFAULT_CONFIG(Type) \
    case ModuleType::Type: \
        return  ModuleTypeTraits<ModuleType::Type>::config{};

namespace Module {
    inline json getDefaultConfig(ModuleType type){
        switch(type){
        HANDLE_DEFAULT_CONFIG(Oscillator)
        HANDLE_DEFAULT_CONFIG(PolyOscillator)
        default:
            return json::object();
        }
    }
}


#endif // __HPP_CONFIGS_ALL_MODULES