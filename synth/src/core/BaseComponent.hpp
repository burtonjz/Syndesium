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
 
#ifndef __BASE_COMPONENT_HPP_
#define __BASE_COMPONENT_HPP_

#include "types/ParameterType.hpp"
#include "types/ComponentType.hpp"
#include "params/ModulationParameter.hpp"
#include "containers/AtomicFloat.hpp"
#include "containers/RTMap.hpp"
#include <nlohmann/json.hpp>

#include <memory>

// forward declarations (needed to support modulation derived classes)
class ParameterMap ; 
class BaseModulator ;

using  ModulationData = RTMap<ModulationParameter, AtomicFloat, N_MODULATION_PARAMETERS> ;
using json = nlohmann::json ;
using ComponentId = int ;

class BaseComponent {
protected:
    ComponentId id_ ;
    ComponentType type_ ;
    std::unique_ptr<ParameterMap> parameters_ ; 

public:
    BaseComponent(ComponentId id = -1, ComponentType type = ComponentType::Unknown);

    virtual ~BaseComponent();
    
    ComponentId getId() const { return id_ ; }
    ComponentType getType() const { return type_ ; }
    ParameterMap* getParameters() { return parameters_.get() ;}

    bool setParameterValue(ParameterType t, const json& value);
    virtual void setParameterModulation(ParameterType p, BaseModulator* m, ModulationData d = {} );
    virtual void removeParameterModulation(ParameterType p);

    // this function runs modulation on all internal parameters
    void updateParameters();
};

#endif // __BASE_COMPONENT_HPP_