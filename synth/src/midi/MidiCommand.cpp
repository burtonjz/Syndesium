/*
 * Copyright (C) 2026 Jared Burton
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

#include "midi/MidiCommand.hpp"

const std::string midiCommand2String(MidiCommand c){
    switch(c){
    case MidiCommand::MIDI_CMD_NOTE_OFF: return "MIDI_CMD_NOTE_OFF";
    case MidiCommand::MIDI_CMD_NOTE_ON: return "MIDI_CMD_NOTE_ON";
    case MidiCommand::MIDI_CMD_NOTE_PRESSURE: return "MIDI_CMD_NOTE_PRESSURE";
    case MidiCommand::MIDI_CMD_CONTROL: return "MIDI_CMD_CONTROL";
    case MidiCommand::MIDI_CMD_PROGRAM: return "MIDI_CMD_PROGRAM";
    case MidiCommand::MIDI_CMD_CHANNEL_PRESSURE: return "MIDI_CMD_CHANNEL_PRESSURE";
    case MidiCommand::MIDI_CMD_PITCHBEND: return "MIDI_CMD_PITCHBEND";
    default: return "";
    }
}
