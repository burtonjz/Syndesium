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