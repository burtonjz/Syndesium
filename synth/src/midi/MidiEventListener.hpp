#ifndef __MIDI_OBSERVER_HPP_
#define __MIDI_OBSERVER_HPP_

#include "midi/MidiNote.hpp"

class MidiEventListener {
public:
    virtual ~MidiEventListener() = default ;

    virtual void onKeyPressed(const ActiveNote* note, bool rePress = false) ;
    virtual void onKeyReleased(ActiveNote anote) ; 
    virtual void onKeyOff(ActiveNote anote) ;
    virtual void onPitchbend(uint16_t pitchbend );
    // virtual void onMidiControlEvent(MidiControlMessage messageType, uint8_t value);
    
};

#endif // __MIDI_OBSERVER_HPP_