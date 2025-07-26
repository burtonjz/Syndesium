#include "midi/MidiController.hpp"
#include "midi/MidiCommand.hpp"
                  
#include <iostream>
#include <cmath>

// static functions

void MidiController::computePitchbendScaleFactor(){
    float shiftValue ;
    for ( int i = 0; i < 16384; ++i ){
        shiftValue = ( i - 8192.0 ) / 16383.0 * CONFIG_PITCHBEND_MAX_SHIFT * 2.0 ;
        pitchbendScaleFactor_[i] = std::pow(2.0f, shiftValue / 12.0f );
    }
}

std::array<double,16384> MidiController::pitchbendScaleFactor_ ;

MidiController::MidiController(MidiState* state):
    state_(state)
{}

void MidiController::initialize(){
    MidiNote::initialize(); // precompute note frequencies
    MidiController::computePitchbendScaleFactor() ; // precompute pitchbend
}

void MidiController::onMidiEvent(double deltaTime, std::vector<unsigned char> *message, void *userData){
    MidiController* self = static_cast<MidiController*>(userData);
    self->processMessage(deltaTime, message);    
}

void MidiController::processMessage(double deltaTime, std::vector<unsigned char> *message){    
    std::cout << "MIDI Message: " << std::endl ;
    MidiCommand command = static_cast<MidiCommand>((*message)[0] & 0xF0) ;
    int channel         = static_cast<int>((*message)[0] & 0x0F) ;

    std::cout << "\tcommand = " << static_cast<int>(command) 
        << "; channel = " << channel 
        << std::endl ;

    switch(command){
        case MidiCommand::MIDI_CMD_NOTE_OFF:
            state_->processMsgNoteOff((*message)[1],(*message)[2]);
            break ;
        case MidiCommand::MIDI_CMD_NOTE_ON:
            state_->processMsgNoteOn((*message)[1],(*message)[2]);
            break ;
        case MidiCommand::MIDI_CMD_NOTE_PRESSURE:
            state_->processMsgNotePressure((*message)[1],(*message)[2]);
            break ;
        case MidiCommand::MIDI_CMD_CONTROL:
            state_->processMsgControl((*message)[1],(*message)[2]);
            break ;
        case MidiCommand::MIDI_CMD_PROGRAM:
            state_->processMsgProgram((*message)[1]);
            break ;
        case MidiCommand::MIDI_CMD_CHANNEL_PRESSURE:
            state_->processMsgChannelPressure((*message)[1]);
            break ;
        case MidiCommand::MIDI_CMD_PITCHBEND:
            state_->processMsgPitchbend(((*message)[2] << 7 ) | (*message)[1] );
            break ;
        default:
            break ;
    }
}

