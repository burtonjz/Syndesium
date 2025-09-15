#ifndef __MIDI_STATE_HPP_
#define __MIDI_STATE_HPP_

#include "containers/RTMap.hpp"
#include "midi/MidiEventHandler.hpp"
#include "midi/MidiNote.hpp"

#include <algorithm>
#include <iostream>

class MidiState {
private:
    std::vector<MidiEventHandler*> handlers_ ; 
    KeyMap notes_ ;
    float pitchbend_ ;

public:
    MidiState():
        handlers_(),
        notes_(),
        pitchbend_(0.0f)
    {}

    void addHandler(MidiEventHandler* handler){
        if (std::find(handlers_.begin(), handlers_.end(), handler) != handlers_.end()) return ;
        handlers_.push_back(handler);
    }

    void removeHandler(MidiEventHandler* handler){
        auto it = std::find(handlers_.begin(), handlers_.end(), handler);
        if ( it != handlers_.end() ){
            handlers_.erase(it);
        }
    }

    // gettrrs/setters
    const MidiNote* getNote(int midiNote){
        auto it = notes_.find(midiNote);        
        if ( it == notes_.end() ) return nullptr ;
        return &notes_[midiNote] ;
    }

    // processing messages
    void processMsgNoteOn(int midiNote, int velocity){
        std::cout << "\tMSG_ON [" << midiNote << " " << velocity << "]" << std::endl ;
        std::cout << "notifying " << handlers_.size() << " handler(s) " << std::endl ; 
        notes_[midiNote].setMidiNote(midiNote);
        notes_[midiNote].setMidiVelocity(velocity);
        notes_[midiNote].setStatus(true);
        for ( auto* h : handlers_ ){
            h->handleKeyPressed(notes_[midiNote]);
        }
    }

    void processMsgNoteOff(int midiNote, int velocity){
        std::cout << "\tMSG_OFF [" << midiNote << " " << velocity << "]" << std::endl ;
        notes_[midiNote].setStatus(false);
        for ( auto* h : handlers_ ){
            h->handleKeyReleased(notes_[midiNote]);
        }
        notes_.erase(midiNote);
    }

    void processMsgPitchbend(float pitchbend){
        pitchbend_ = pitchbend ;
        for ( auto* h : handlers_ ){
            h->handlePitchbend(pitchbend_);
        }
    }

    void processMsgNotePressure(int midiNote, int pressure){

    }

    void processMsgControl(int ctrlID, int ctrlValue){

    }

    void processMsgProgram(int program){

    }

    void processMsgChannelPressure(int pressure){

    }

    void reset(){
        handlers_.clear();
        notes_.clear();
        pitchbend_ = 0.0f ;
    }

};

#endif // __MIDI_STATE_HPP_