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

#ifndef __ENGINE_HPP_
#define __ENGINE_HPP_

#include <atomic>

#include <map>
#include <rtmidi/RtMidi.h>
#include <rtaudio/RtAudio.h>

#include "midi/MidiController.hpp"
#include "midi/MidiEventHandler.hpp"
#include "midi/MidiState.hpp"
#include "types/Waveform.hpp"
#include "signal/SignalController.hpp"
#include "core/ComponentManager.hpp"
#include "core/ComponentFactory.hpp"

#include <nlohmann/json.hpp> 

using json = nlohmann::json ;

class Engine {
private:
    // audio
    RtAudio dac_ ;
    std::map<int,std::string> availableAudioDevices_ ; // list available devices
    int selectedAudioOutput_ ; // selected device id
    double dt_ ; // 1 / sampleRate

    // midi
    RtMidiIn midiIn_ ;
    std::map<int,std::string> availableMidiPorts_ ;
    int selectedMidiPort_ ;  

    // midi manipulation
    MidiState        midiState_ ;
    MidiEventHandler midiDefaultHandler_ ;

public:
    // set a flag for all threads to stop on
    static std::atomic<bool> stop_flag ;
    static void signalHandler(int signum);

    // publically available interfaces
    ComponentManager componentManager ;
    ComponentFactory componentFactory ;
    SignalController signalController ;
    MidiController   midiController ;

    // static thread functions
    static void startAudio(Engine* engine);
    void stopAudio();

    static int audioCallback(void *outputBuffer, void *inputBuffer, unsigned int nBufferFrames,
                              double streamTime, RtAudioStreamStatus status, void *userData );
    static void audioCleanup(RtAudio* dac, RtAudioErrorType error);

    static void startMidi(Engine* engine);
    void stopMidi();

    // Constructor
    Engine();

    // Engine Block Functions
    void initialize() ;
    void run() ;
    void stop() ;

    // Getters & Setters
    RtAudio* getDac() ;
    RtMidiIn* getMidiIn() ;
    MidiController* getMidiController() ;

    double getDeltaTime() const;

    int getAudioDeviceId() const ;
    void setAudioDeviceId(int deviceId);

    int getMidiDeviceId() const ;
    void setMidiDeviceId(int deviceId);

    void setup();
    void destroy();

    void setOscillatorWaveform(int oscillatorID, Waveform wf );

    const std::map<int,std::string> getAvailableMidiDevices() const ;
    const std::map<int,std::string> getAvailableAudioDevices() const ;

    // connection logic
    bool setMidiConnection(MidiEventHandler* handler, MidiEventListener* listener);
    bool removeMidiConnection(MidiEventHandler* handler, MidiEventListener* listener);
    bool registerMidiHandler(MidiEventHandler* handler);
    bool unregisterMidiHandler(MidiEventHandler* handler);



    
    



};

#endif // __ENGINE_HPP_