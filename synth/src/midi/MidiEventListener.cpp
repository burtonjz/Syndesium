#include "midi/MidiEventListener.hpp"

void MidiEventListener::onKeyPressed(const ActiveNote* note, bool repress){
} 
void MidiEventListener::onKeyReleased(ActiveNote anote){
}
void MidiEventListener::onKeyOff(ActiveNote anote){
}
void MidiEventListener::onPitchbend(uint16_t pitchbend ){
}

// void MidiEventListener::onMidiControlEvent(MidiControlMessage messageType, uint8_t value){
// }