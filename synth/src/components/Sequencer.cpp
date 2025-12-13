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

#include "components/Sequencer.hpp"
#include "types/ComponentType.hpp"
#include "types/ParameterType.hpp"


Sequencer::Sequencer(ComponentId id, SequencerConfig cfg):
    BaseComponent(id, ComponentType::Sequencer),
    currentTime_(0)
{
    parameters_->add<ParameterType::STATUS>(true, false);
    parameters_->add<ParameterType::AMPLITUDE>(cfg.velocity / 127.0f, true);
    parameters_->add<ParameterType::MAX_VALUE>(cfg.length, false, 0, cfg.max_length);

    initializeSequence();
}


bool Sequencer::shouldKillNote(const ActiveNote& anote) const {
    return activeNotes_.find(anote.note.getMidiNote()) == activeNotes_.end() ;
}

void Sequencer::onTick(float dt){
    if ( !parameters_->getParameter<ParameterType::STATUS>()->getValue() ) return ;

    int bpm = sequence_->getBpm() ;
    float maxBeats = parameters_->getParameter<ParameterType::MAX_VALUE>()->getInstantaneousValue();
    float loopedBeat = std::fmod((currentTime_ * bpm ) / 60.0f , maxBeats);

    activeNotes_ = sequence_->getActiveNotes(loopedBeat);
    uint8_t velocity = parameters_->getParameter<ParameterType::AMPLITUDE>()->getInstantaneousValue() * 127 ;

    for ( uint8_t n : activeNotes_ ){
        if ( !isNoteActive(n) ) {
            MidiNote m = MidiNote(n, velocity, true);
            ActiveNote anote = {m, 0};
            MidiEvent e = {MidiEvent::Type::NotePressed, anote, false };
            queue_.push(e);
        }
    }    

    currentTime_ += dt ;
}

