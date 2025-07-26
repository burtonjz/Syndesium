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