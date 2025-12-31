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

#include "components/MonophonicFilter.hpp"
#include "midi/MidiEventHandler.hpp"


MonophonicFilter::MonophonicFilter(ComponentId id, [[maybe_unused]] MonophonicFilterConfig cfg):
    BaseComponent(id, ComponentType::MonophonicFilter)
{}

void MonophonicFilter::onKeyPressed(const ActiveNote* note, bool rePressed){
    uint8_t midiNote = note->note.getMidiNote() ;
    noteStack_.push_back(midiNote);
    
    if (noteStack_.size() > 1) {
        auto lastNote = notes_[noteStack_[noteStack_.size() - 2]];
        lastNote.note.setStatus(false);
        MidiEventHandler::onKeyReleased(lastNote);
    } 
    
    MidiEventHandler::onKeyPressed(note, rePressed);
}

void MonophonicFilter::onKeyReleased(ActiveNote anote){
    uint8_t midiNote = anote.note.getMidiNote();
    
    auto it = std::find(noteStack_.begin(), noteStack_.end(), midiNote);
    
    if ( it == noteStack_.end() ){
        return ;
    }

    bool isActiveNote = ( it == noteStack_.end() - 1 );
    noteStack_.erase(it);

    if ( isActiveNote ){
        // release and trigger last
        MidiEventHandler::onKeyReleased(anote);

        if ( !noteStack_.empty() ){
            uint8_t nextNote = noteStack_.back();
            ActiveNote* nextANote = &notes_[nextNote];
            nextANote->note.setStatus(true);
            MidiEventHandler::onKeyPressed(nextANote, false);
        }
    } 
    
}