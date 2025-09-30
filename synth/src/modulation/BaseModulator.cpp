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

#include "modulation/BaseModulator.hpp"
#include "params/ParameterMap.hpp"

BaseModulator::BaseModulator(ModulatorType typ):
        type_(typ),
        parameters_(std::make_unique<ParameterMap>())
    {}

ModulatorType BaseModulator::getType() const {
    return type_ ;
}

ParameterMap* BaseModulator::getParameters(){
    return parameters_.get() ;
}

void BaseModulator::setParameterModulation(ParameterType p, BaseModulator* m, ModulationData d ){
    if ( ! parameters_ ) return ;
    if ( d.empty() ){
        auto required = m->getRequiredModulationParameters();
        for ( auto mp : required ){
            d[mp];
        }
    }
    parameters_->setModulation(p,m,d);
}

void BaseModulator::tick(){
    if (parameters_) parameters_->modulate() ;
}