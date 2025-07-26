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

