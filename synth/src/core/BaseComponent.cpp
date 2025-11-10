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

#include "core/BaseComponent.hpp"
#include "core/BaseModule.hpp"
#include "params/ParameterMap.hpp"
#include <unordered_set>

BaseComponent::BaseComponent(ComponentId id, ComponentType type):
    id_(id),
    type_(type),
    parameters_(std::make_unique<ParameterMap>())
{}

BaseComponent::~BaseComponent() = default ;

std::unordered_set<BaseModule*>& BaseComponent::getModulationInputs(){
    return modulationModules_ ;
}

bool BaseComponent::setParameterValue(ParameterType t, const json& value){
    return parameters_->setValueDispatch(t,value); 
}

void BaseComponent::setParameterModulation(ParameterType p, BaseModulator* m, ModulationData d ){
    if ( ! parameters_ ) return ;

    // if the modulator is stateful (also a module), track it for signal chain
    BaseModule* module = dynamic_cast<BaseModule*>(m);
    if ( module ){
        modulationModules_.insert(module);
    }
    
    onSetParameterModulation(p,m,d);
}

void BaseComponent::removeParameterModulation(ParameterType p){
    if ( ! parameters_ ) return ;

    // if the modulator is stateful (also a module), remove tracking
    BaseModulator* modulator = parameters_->getParameter(p)->getModulator();
    if ( BaseModule* module = dynamic_cast<BaseModule*>(modulator) ){
        modulationModules_.erase(module);
    }

    onRemoveParameterModulation(p);
}

void BaseComponent::updateParameters(){
    if (parameters_) parameters_->modulate() ;
}

void BaseComponent::onSetParameterModulation(ParameterType p, BaseModulator* m, ModulationData d ){
    if ( d.empty() && m ){
        auto required = m->getRequiredModulationParameters();
        for ( auto mp : required ){
            d[mp];
        }
    }
    parameters_->getParameter(p)->setModulation(m,d);
}

void BaseComponent::onRemoveParameterModulation(ParameterType p){
    parameters_->getParameter(p)->removeModulation();
}