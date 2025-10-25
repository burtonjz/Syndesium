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

#ifndef __MIDI_EVENT_HANDLER_HPP_
#define __MIDI_EVENT_HANDLER_HPP_

#include "core/BaseComponent.hpp"
#include "midi/MidiNote.hpp"
#include "midi/MidiEventListener.hpp"
#include "midi/MidiEventQueue.hpp"

#include <iostream>
#include <vector>
#include <algorithm>

class MidiEventHandler : public MidiEventListener, public virtual BaseComponent {
protected:
    ActiveNoteMap notes_ ;
    std::vector<MidiEventListener*> listeners_ ;
    MidiEventQueue queue_ ;

    // broadcast functions to notify listeners
    void notifyKeyPressed(ActiveNote* note, bool rePressed = false) {
        for (auto* li : listeners_) li->onKeyPressed(note, rePressed);
    }
    
    void notifyKeyReleased(const ActiveNote& note) {
        for (auto* li : listeners_) li->onKeyReleased(note);
    }
    
    void notifyKeyOff(const ActiveNote& note) {
        for (auto* li : listeners_) li->onKeyOff(note);
    }
    
    void notifyPitchbend(uint16_t pitchbend) {
        for (auto* li : listeners_) li->onPitchbend(pitchbend);
    }

public:
    MidiEventHandler(): BaseComponent() {}
    virtual ~MidiEventHandler() = default ;

    void addListener(MidiEventListener* listener){
        if ( std::find(listeners_.begin(), listeners_.end(), listener) != listeners_.end() ) return ;
        listeners_.push_back(listener);
    }

    void removeListener(MidiEventListener* listener){
        auto it = std::find(listeners_.begin(), listeners_.end(), listener);
        if ( it != listeners_.end() ){
            listeners_.erase(it);
        }
    }

    // all handlers are listeners so that they can chain operations if desired. By default, we simply pass through
    void onKeyPressed(const ActiveNote* note, bool rePressed = false) override {
        MidiEvent e = { MidiEvent::Type::NotePressed, *note, rePressed };
        queue_.push(e);
    }

    void onKeyReleased(ActiveNote anote) override {
        MidiEvent e = { MidiEvent::Type::NoteReleased, anote };
        queue_.push(e);
    }

    void onKeyOff(ActiveNote anote) override {
        MidiEvent e = { MidiEvent::Type::NoteOff, anote };
        queue_.push(e);
    }

    void onPitchbend(uint16_t pitchbend) override {
        notifyPitchbend(pitchbend);
    }

    // handler functions for root-level midi input from midi state
    void handleKeyPressed(const MidiNote note){
        MidiEvent e = { MidiEvent::Type::NotePressed, {note} };
        if ( notes_.find(e.anote.note.getMidiNote()) != notes_.end()){
            e.rePressed = true ;
            queue_.push(e);
            return ;
        } 

        ActiveNote n {note, 0.0f};
        notes_[e.anote.note.getMidiNote()] = n ;
        queue_.push(e);
    };

    void handleKeyReleased(const MidiNote note){
        MidiEvent e = { MidiEvent::Type::NoteReleased, {note} };
        queue_.push(e);
    };

    void handlePitchbend(uint16_t pitchbend){
        notifyPitchbend(pitchbend);
    };

    
    // determines if a note should be killed on tick. Default behavior is to kill a note as soon as note off is received via Midi
    virtual bool shouldKillNote(const ActiveNote& anote) const {
        return !anote.note.getStatus() ;
    }

    
    void processEvents(){
        MidiEvent e ;
        while (queue_.pop(e)){
            auto it = notes_.find(e.anote.note.getMidiNote());
            if ( it == notes_.end() ) continue ;
            ActiveNote& anote = it->second ;
            
            switch (e.type){
            case MidiEvent::Type::NotePressed:
                anote.resetTime();
                anote.note.setStatus(true);
                notifyKeyPressed(&notes_[e.anote.note.getMidiNote()], e.rePressed);
                break ;
            case MidiEvent::Type::NoteReleased:
                anote.resetTime();
                anote.note.setStatus(false);
                notifyKeyReleased(anote);
                break ;
            case MidiEvent::Type::NoteOff:
                notifyKeyOff(anote);
                notes_.erase(it);
                break ;
            }
        }
    }

    void tick(float dt){
        processEvents();

        // check for any notes that need to be released
        for (auto it = notes_.begin(); it != notes_.end(); ++it ){
            if ( shouldKillNote(it->second) ){
                MidiEvent e = {MidiEvent::Type::NoteOff, it->second};
                queue_.push(e);
            } else {
                it->second.updateTime(dt);
            }
        }
    }
};

#endif // __MIDI_EVENT_HANDLER_HPP_