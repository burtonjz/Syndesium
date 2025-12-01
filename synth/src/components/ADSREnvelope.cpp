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

#include "components/ADSREnvelope.hpp"
#include "params/ModulationParameter.hpp"
#include "params/ParameterMap.hpp"
#include "types/ParameterType.hpp"
ADSREnvelope::ADSREnvelope(ComponentId id, ADSREnvelopeConfig cfg):
    BaseComponent(id,ComponentType::ADSREnvelope),
    BaseModulator()
{
    parameters_->add<ParameterType::ATTACK>(cfg.attack, true);
    parameters_->add<ParameterType::DECAY>(cfg.decay, true);
    parameters_->add<ParameterType::SUSTAIN>(cfg.sustain, true);
    parameters_->add<ParameterType::RELEASE>(cfg.release, true);

    requiredParams_ = {
        ModulationParameter::MIDI_NOTE, 
        ModulationParameter::INITIAL_VALUE
    };
}

double ADSREnvelope::modulate([[maybe_unused]] double value, ModulationData* mData) const {
    double output = 0.0 ;

    // check required data
    if ( !mData ) return output ; 
    if ( mData->find(ModulationParameter::MIDI_NOTE)     == mData->end() ) return output ;

    if ( mData->find(ModulationParameter::INITIAL_VALUE) == mData->end() ){
        (*mData)[ModulationParameter::INITIAL_VALUE] = 0.0f ;
    }
    if ( mData->find(ModulationParameter::OUTPUT_1)    == mData->end() ){ 
        (*mData)[ModulationParameter::OUTPUT_1] = 0.0f ;    
    }

    uint8_t midiNote = static_cast<uint8_t>((*mData)[ModulationParameter::MIDI_NOTE].get()) ;
    auto it = notes_.find(midiNote) ;
    if ( it == notes_.end() ){ return output ; }

    float start_level = (*mData)[ModulationParameter::INITIAL_VALUE].get();
    
    if ( it->second.note.getStatus() ){
        // then note is pressed
        float attack = parameters_->getParameter<ParameterType::ATTACK>()->getInstantaneousValue() ;
        float decay = parameters_->getParameter<ParameterType::DECAY>()->getInstantaneousValue() ;
        float sustain = parameters_->getParameter<ParameterType::SUSTAIN>()->getInstantaneousValue() ;
        
        if ( it->second.time <= attack ) {
            output = start_level + ( 1.0f - start_level ) * (it->second.time / attack) ;
        } else if ( it->second.time <= (attack + decay) ) {
            output = 1.0f - (1.0f - sustain) * ((it->second.time - attack) / decay );
        } else {
            output = sustain ;
        }
    } else {
        float release = parameters_->getParameter<ParameterType::RELEASE>()->getInstantaneousValue() ;
        if ( it->second.time >= release ){
            output = 0.0 ;
        } else {
            output = start_level * (1 - (it->second.time / release)) ;
        }
    }

    (*mData)[ModulationParameter::OUTPUT_1].set(output) ;
    return output ;
}

bool ADSREnvelope::shouldKillNote(const ActiveNote& note) const {
    float release = parameters_->getParameter<ParameterType::RELEASE>()->getInstantaneousValue() ;
    return ( !note.note.getStatus() && note.time > release ) ;
}