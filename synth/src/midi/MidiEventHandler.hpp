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

    /**
     * The below functions broadcast handler events to downstream objects
     * Note: MidiEventHandlers will broadcast to chained handlers such that 
     * each handler is able to act as if it is receiving raw midi. This allows
     * a chained midi event handler to act in isolation in determining when a
     * note should be killed.

     * This means midiEventHandlers receive OFF as RELEASE, where as the end node
     * midiEventListeners are the only ones that actually receive OFF events

     * Importantly, this means that midi handlers that modify the release time should
     * be at the end of the chain, so that other handlers in the chain don't mask the 
     * release time from downstream modules (see Polyphonic Oscillator implementation for 
     * an example into problems that can arise)
    */

    // notify downstream listeners of a key press event
    void notifyKeyPressed(ActiveNote* note, bool rePressed = false) {
        for ( auto* li : listeners_ ){
            if (auto h = dynamic_cast<MidiEventHandler*>(li)){
                h->handleKeyPressed(note->note);
            } else {
                li->onKeyPressed(note, rePressed);
            }
        }
    }
    
    // notify downstream listeners of a key release event (only if not handler)
    void notifyKeyReleased(const ActiveNote& note) {
        for ( auto* li : listeners_ ){
            // only notify end node listeners of release events
            if ( !dynamic_cast<MidiEventHandler*>(li) ){
                li->onKeyReleased(note);
            }
        } 
    }
    
    // notify downstream listeners of a key off event (release if handler)
    void notifyKeyOff(const ActiveNote& note) {
        for ( auto* li : listeners_ ){
            if ( auto h = dynamic_cast<MidiEventHandler*>(li) ){
                h->handleKeyReleased(note.note);
            } else {
                li->onKeyOff(note);
            }
        } 
    }
    
    void notifyPitchbend(uint16_t pitchbend) {
        for ( auto* li : listeners_ ) li->onPitchbend(pitchbend);
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

    // all handlers are listeners so that they can chain operations if desired. 
    // The below functions can be overridded to provide specialized operations when cascading events downwards
    void onKeyPressed(const ActiveNote* note, bool rePressed = false) override {
        MidiEvent e = { MidiEvent::Type::NotePressed, *note, rePressed };
        queue_.push(e);
    }

    void onKeyReleased(ActiveNote anote) override {
        anote.resetTime();
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
        ActiveNote n{note};
    
        bool rePress = false ;
        if ( notes_.find(note.getMidiNote()) != notes_.end()){
            rePress = true ;
        }
        onKeyPressed(&n, rePress);
    };

    void handleKeyReleased(const MidiNote note){
        if ( notes_.find(note.getMidiNote()) == notes_.end() ) return ;
        auto anote = notes_[note.getMidiNote()] ;
        anote.note = note ;
        onKeyReleased(anote);
    };

    void handlePitchbend(uint16_t pitchbend){
        onPitchbend(pitchbend);
    };

    
    // determines if a note should be killed on tick. Default behavior is to kill a note as soon as note off is received via Midi
    virtual bool shouldKillNote(const ActiveNote& anote) const {
        return !anote.note.getStatus() ;
    }

    
    void processEvents(){
        MidiEvent e ;
        while (queue_.pop(e)){    
            ActiveNote note = e.anote ;
            switch (e.type){
            case MidiEvent::Type::NotePressed:
                notes_[e.anote.note.getMidiNote()] = note ;
                notifyKeyPressed(&notes_[e.anote.note.getMidiNote()], e.rePressed);
                break ;
            case MidiEvent::Type::NoteReleased:
                notes_[e.anote.note.getMidiNote()] = note ;
                notifyKeyReleased(note);
                break ;
            case MidiEvent::Type::NoteOff:
                notifyKeyOff(note);
                notes_.erase(note.note.getMidiNote());
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