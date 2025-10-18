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

class MidiEventHandler : public virtual BaseComponent {
protected:
    ActiveNoteMap notes_ ;
    std::vector<MidiEventListener*> listeners_ ;
    MidiEventQueue queue_ ;

public:
    MidiEventHandler():
        BaseComponent()
    {}

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

    /**
     * @brief for use in audio thread for thread-safe handling of midi events
     * 
     */
    void processEvents(){
        MidiEvent e ;
        while (queue_.pop(e)){
            auto it = notes_.find(e.anote.note.getMidiNote());
            if ( it == notes_.end() ) continue ;
            ActiveNote& anote = it->second ;
            
            switch (e.type){
            case MidiEvent::Type::NotePressed:
                // handle repress
                if (e.rePressed){
                    anote.resetTime();
                    anote.note.setStatus(true);
                    for (auto* li: listeners_ ) li->onKeyPressed(&notes_[e.anote.note.getMidiNote()], true);
                } else {
                    anote.resetTime();
                    anote.note.setStatus(true);
                    for (auto* li: listeners_ ) li->onKeyPressed(&anote);
                }
                break ;
            case MidiEvent::Type::NoteReleased:
                anote.resetTime();
                anote.note.setStatus(false);
                for (auto* li: listeners_ ) li->onKeyReleased(anote);
                break ;
            case MidiEvent::Type::NoteOff:
                for (auto* li: listeners_ ) li->onKeyOff(anote);
                notes_.erase(it);
                break ;
            }
        }
    }

    /**
     * @brief On a key press from the Midi system
     * 
     * @param note 
     */
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
        return ;
    };

    /**
     * @brief On a key released from a midi system
     * 
     * @param note 
     */
    virtual void handleKeyReleased(const MidiNote note){
        MidiEvent e = { MidiEvent::Type::NoteReleased, {note} };
        queue_.push(e);
    };

    virtual void handlePitchbend(uint16_t pitchbend){
        for (auto* li : listeners_) li->onPitchbend(pitchbend);
    };

    /**
     * @brief determines if a note should be killed on tick. Default behavior is to kill a note as soon as note off is received via Midi
     * 
     * @param note ActiveNote
     */
    virtual bool shouldKillNote(const ActiveNote& anote) const {
        return !anote.note.getStatus() ;
    }

    void tick(float dt){
        processEvents();

        // also, the handler will check if a note should be killed (e.g., if enough time has passed that a release stage is concluded)
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