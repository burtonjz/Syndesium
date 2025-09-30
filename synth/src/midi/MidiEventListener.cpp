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