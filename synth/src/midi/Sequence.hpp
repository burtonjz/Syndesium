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

#ifndef SEQUENCE_HPP_
#define SEQUENCE_HPP_

#include "types/SequenceData.hpp"
#include "types/ParameterType.hpp"
#include "params/Parameter.hpp"
#include "params/ParameterMap.hpp"
#include "core/BaseComponent.hpp"
#include <stdexcept>

class Sequence {
protected:
    Parameter<ParameterType::BPM>* bpm_ ;
    SequenceData sequence_ ;

public:
    Sequence(BaseComponent* component):
        bpm_(nullptr),
        sequence_()
    {
        if ( !component ){
            throw std::runtime_error("Sequence: component cannot be null");
        }

        auto params = component->getParameters();

        if ( !params ){
            throw std::runtime_error("Sequence: component has no parameter map");
        }

        if ( !params->getParameter<ParameterType::BPM>() ){
            params->add<ParameterType::BPM>(GET_PARAMETER_TRAIT_MEMBER(ParameterType::BPM, defaultValue), true);
        }

        bpm_ = params->getParameter<ParameterType::BPM>() ;

    }

    ~Sequence() = default ;

    SequenceData& getSequence(){
        return sequence_ ;
    }

    const SequenceData& getSequence() const {
        return sequence_ ;
    }

    int getBpm() const {
        return bpm_->getInstantaneousValue();
    }

    std::set<uint8_t> getActiveNotes(float currentBeat){
        return sequence_.getActiveNotes(currentBeat);
    }
};

#endif // SEQUENCE_HPP_