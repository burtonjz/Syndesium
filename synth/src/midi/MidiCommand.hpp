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

#ifndef MIDI_COMMAND_HPP_
#define MIDI_COMMAND_HPP_

enum class MidiCommand {
    MIDI_CMD_NOTE_OFF         = 0x80,
    MIDI_CMD_NOTE_ON          = 0x90,
    MIDI_CMD_NOTE_PRESSURE    = 0xA0,
    MIDI_CMD_CONTROL          = 0xB0,
    MIDI_CMD_PROGRAM          = 0xC0,
    MIDI_CMD_CHANNEL_PRESSURE = 0xD0,
    MIDI_CMD_PITCHBEND        = 0xE0,
};

#endif // MIDI_COMMAND_HPP_