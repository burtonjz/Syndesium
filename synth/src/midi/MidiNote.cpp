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

#include "midi/MidiNote.hpp"
#include <cmath>

// static functions/params
void MidiNote::initialize(){
    for ( uint8_t i = 0 ; i < 128 ; ++i ){
        frequencies_[i] = std::pow(2.0, (static_cast<double>(i) - 69.0) / 12.0) * 440.0 ;
    }
}
std::array<double, 128> MidiNote::frequencies_ ;

MidiNote::MidiNote():
    midiNote_(0),
    midiVelocity_(0),
    midiOn_(false)
{}

MidiNote::MidiNote(uint8_t midiNote, uint8_t midiVelocity, bool midiOn):
    midiNote_(midiNote),
    midiVelocity_(midiVelocity),
    midiOn_(midiOn)
{}

uint8_t MidiNote::getMidiNote() const {
    return midiNote_ ;
}

void MidiNote::setMidiNote(uint8_t note ){
    if ( note > 127 ) midiNote_ = 127 ;
    else midiNote_ = note ;
}

uint8_t MidiNote::getMidiVelocity() const {
    return midiVelocity_ ;
}

void MidiNote::setMidiVelocity(uint8_t velocity){
        if ( velocity > 127 ) midiVelocity_ = 127 ;
        else midiVelocity_ = velocity ;
}

bool MidiNote::getStatus() const {
    return midiOn_ ;
}

void MidiNote::setStatus(bool midiOn){
    if ( midiOn_ != midiOn ){
        midiOn_ = midiOn ;
    }
}

double MidiNote::getFrequency(uint8_t note){
    if ( note > 127 ) return frequencies_[127] ;
    return frequencies_[note] ;
}

double MidiNote::getFrequency() const {
    return frequencies_[midiNote_] ;
}

