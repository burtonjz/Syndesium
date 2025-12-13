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
#include "midi/Sequence.hpp"
#include "types/SequenceData.hpp"

#include <memory>
#include <stdexcept>
#include <vector>
#include <algorithm>

class MidiEventHandler : public MidiEventListener, public virtual BaseComponent {
protected:
    std::array<ActiveNote,128> notes_ ;
    std::array<uint8_t,128> noteIndices_ ;
    uint8_t activeCount_ = 0 ;

    std::vector<MidiEventListener*> listeners_ ;
    MidiEventQueue queue_ ;

    std::unique_ptr<Sequence> sequence_ ;

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
    MidiEventHandler():
        BaseComponent(),
        sequence_(nullptr)
    {}
    
    virtual ~MidiEventHandler() = default ;

    bool hasSequence() const {
        return sequence_.get() ;
    }

    SequenceData& getSequence() const {
        if ( !sequence_ ){
            throw std::runtime_error("MidiEventHandler: cannot get sequence for a non-sequenceable component.");
        } ;
        return sequence_.get()->getSequence();
    }

    void initializeSequence(){
        if ( sequence_ ){
            std::runtime_error("MidiEventHandler: sequence can only be initialized once.");
        }

        sequence_ = std::make_unique<Sequence>(this);
    }

    bool addSequenceNote(SequenceNote n){
        if ( !sequence_ ) return false ;
        if ( n.velocity )
        sequence_->getSequence().addNote(n);
        return true ;
    }

    bool removeSequenceNote(SequenceNote n){
        if ( !sequence_ ) return false ;
        sequence_->getSequence().removeNote(n);
        return true ;
    }

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

    std::vector<MidiEventListener*>& getListeners(){
        return listeners_ ;
    }

    bool isNoteActive(uint8_t n) const {
        for ( uint8_t i = 0 ; i < activeCount_; ++i ){
            if (noteIndices_[i] == n ) return true ;
        }
        return false ;
    }

    void activateNote(const ActiveNote& anote){
        uint8_t midiNote = anote.note.getMidiNote() ;
        notes_[midiNote] = anote ;

        if ( isNoteActive(midiNote) ){
            return ;
        }
        noteIndices_[activeCount_++] = midiNote ;
    }

    void deactivateNote(uint8_t n){
        for ( uint8_t i = 0 ; i < activeCount_; ++i ){
            if ( noteIndices_[i] == n ){
                noteIndices_[i] = noteIndices_[--activeCount_];
                break ;
            }
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
        if ( isNoteActive(note.getMidiNote()) ){
            rePress = true ;
        }
        onKeyPressed(&n, rePress);
    };

    void handleKeyReleased(const MidiNote note){
        auto anote = notes_[note.getMidiNote()] ;
        if ( ! isNoteActive(note.getMidiNote()) ) return ;
        
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
            ActiveNote anote = e.anote ;
            uint8_t midiNote = anote.note.getMidiNote() ;
            switch (e.type){
            case MidiEvent::Type::NotePressed:
                activateNote(anote);
                notifyKeyPressed(&notes_[midiNote], e.rePressed);
                break ;
            case MidiEvent::Type::NoteReleased:
                notes_[e.anote.note.getMidiNote()] = anote ;
                notifyKeyReleased(anote);
                break ;
            case MidiEvent::Type::NoteOff:
                notifyKeyOff(anote);
                deactivateNote(midiNote);
                break ;
            }
        }
    }

    void tick(float dt){
        processEvents();

        onTick(dt); // child class implementations

        for ( uint8_t i = 0; i < activeCount_ ; ++i ){
            ActiveNote& note = notes_[noteIndices_[i]];
            if ( shouldKillNote(note) ){
                MidiEvent e = {MidiEvent::Type::NoteOff, note};
                queue_.push(e);
            } else {
                note.updateTime(dt);
            }
        }
    }

    virtual void onTick([[maybe_unused]] float dt){
    }
};

#endif // __MIDI_EVENT_HANDLER_HPP_