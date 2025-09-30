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

#include "modulation/ADSREnvelope.hpp"
#include "types/ModulatorType.hpp"
#include "params/ModulationParameter.hpp"
#include "types/ParameterType.hpp"
#include <iostream>

ADSREnvelope::ADSREnvelope(ADSREnvelopeConfig cfg):
    BaseModulator(staticType)
{
    parameters_.add<ParameterType::ATTACK>(cfg.attack, true);
    parameters_.add<ParameterType::DECAY>(cfg.decay, true);
    parameters_.add<ParameterType::SUSTAIN>(cfg.sustain, true);
    parameters_.add<ParameterType::RELEASE>(cfg.release, true);

    requiredParams_ = {
        ModulationParameter::MIDI_NOTE, 
        ModulationParameter::INITIAL_VALUE,
        ModulationParameter::LAST_VALUE
    };
}

double ADSREnvelope::modulate(double value, ModulationData* mData) const {
    double output = value ;

    // check required data
    if ( !mData ) return value ; 
    if ( mData->find(ModulationParameter::MIDI_NOTE)     == mData->end() ){ return value ; }
    if ( mData->find(ModulationParameter::INITIAL_VALUE) == mData->end() ){ return value ; }
    if ( mData->find(ModulationParameter::LAST_VALUE)    == mData->end() ){ return value ; }
    
    uint8_t midiNote = static_cast<uint8_t>((*mData)[ModulationParameter::MIDI_NOTE].get()) ;

    auto it = notes_.find(midiNote) ;
    if ( it == notes_.end() ){ return value ; }

    float start_level = (*mData)[ModulationParameter::INITIAL_VALUE].get();
    
    if ( it->second.note.getStatus() ){
        // then note is pressed
        float attack = parameters_.getInstantaneousValue<ParameterType::ATTACK>() ;
        if ( it->second.time <= attack ) {
            output = start_level + ( value - start_level ) * (it->second.time / attack) ;
        }
    } else {
        float release = parameters_.getInstantaneousValue<ParameterType::RELEASE>() ;
        if ( it->second.time >= release ){
            output = 0.0 ;
        } else {
            output = start_level * (1 - (it->second.time / release)) ;
        }
    }

    (*mData)[ModulationParameter::LAST_VALUE].set(output) ;
    return output ;
}

bool ADSREnvelope::shouldKillNote(const ActiveNote& note) const {
    float release = parameters_.getInstantaneousValue<ParameterType::RELEASE>() ;
    return ( !note.note.getStatus() && note.time > release ) ;
}