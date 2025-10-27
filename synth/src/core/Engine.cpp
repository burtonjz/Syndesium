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

#include "core/Engine.hpp"
#include "config/Config.hpp"
#include "api/ApiHandler.hpp"
#include "midi/MidiEventHandler.hpp"
#include "midi/MidiEventListener.hpp"
#include "types/Waveform.hpp"
#include "dsp/math.hpp"
#include "midi/MidiController.hpp"

#include <chrono>
#include <csignal>
#include <iostream>
#include <netinet/in.h>
#include <sys/socket.h>
#include <fcntl.h>
#include <thread>
#include <unistd.h>


// static members and functions
std::atomic<bool> Engine::stop_flag{false};

void Engine::signalHandler(int signum){
    std::cout << "Caught signal " << signum << ", stopping threads...\n" ;
    stop_flag = true ;
}

void Engine::startAudio(Engine* engine){
    unsigned int buffer = Config::get<unsigned int>("audio.buffer_size").value() ;
    unsigned int sampleRate = Config::get<unsigned int>("audio.sample_rate").value() ;

    // now set up the audio stream
    RtAudio* dac = engine->getDac() ;
    RtAudio::StreamParameters parameters ;
    std::map<int,std::string> devices = engine->getAvailableAudioDevices() ;
    int deviceId = engine->getAudioDeviceId();
    RtAudio::DeviceInfo deviceInfo ;
    
    if ( deviceId == 0 || devices.find(deviceId) == devices.end() ){ // invalid device
        deviceInfo = dac->getDeviceInfo(dac->getDefaultInputDevice());
    } else {
        deviceInfo = dac->getDeviceInfo(deviceId);
    }

    parameters.deviceId = deviceInfo.ID ;
    parameters.nChannels = 1 ;
    parameters.firstChannel = 0 ;
    
    // make sure sampling rate from config is valid
    auto &sampleRates = deviceInfo.sampleRates ;
    if ( std::count(sampleRates.begin(), sampleRates.end(), sampleRate) == 0 ){
        std::cout << "configured sample rate of " << sampleRate << " is not supported by device " << deviceInfo.name 
            << ". Setting to device preferred sample rate of " << deviceInfo.preferredSampleRate << "." << std::endl ;
        sampleRate = deviceInfo.preferredSampleRate ;
        Config::set("audio.sample_rate", sampleRate );
    }

    // allow callback function to access engine
    void* userData = static_cast<void*>(engine) ;

    // attempt to open audio stream
    if ( dac->openStream(
        &parameters,
        NULL,
        RTAUDIO_FLOAT64,
        sampleRate,
        &buffer,
        &audioCallback,
        userData
    )){
        audioCleanup(dac, RTAUDIO_UNKNOWN_ERROR);
    }

    // start audio stream
    if ( dac->startStream() ){
        audioCleanup(dac, RTAUDIO_UNKNOWN_ERROR);
    }

    while (!stop_flag) {
        std::this_thread::sleep_for(std::chrono::seconds(1)); // Keep app alive
    }
}

void Engine::stopAudio(){
    if ( dac_.isStreamRunning() ) dac_.stopStream();
    if ( dac_.isStreamOpen() ) dac_.closeStream();
}

int Engine::audioCallback(
    void *outputBuffer, void *inputBuffer, unsigned int nBufferFrames, double streamTime, 
    RtAudioStreamStatus status, void *userData 
){
    Engine* engine = static_cast<Engine*>(userData);
    double* buffer = static_cast<double*>(outputBuffer) ;
    
    engine->signalController.clearBuffer();

    double sample ;
    for ( unsigned int i = 0 ; i < nBufferFrames ; ++i ){
        engine->midiController.tick(engine->getDeltaTime());
        engine->componentManager.runParameterModulation();
        sample = engine->signalController.processFrame();
        buffer[i] = sample ; // buffer[i] = dsp::fastAtan(sample) ;
    }
    
    engine->analysisAudioOut_.push(buffer, nBufferFrames);
    return 0 ;
}

void Engine::audioCleanup(RtAudio* dac, RtAudioErrorType error){
    if (error) std::cout << '\n' << dac->getErrorText() << '\n' << std::endl ;
    if (dac->isStreamOpen()) dac->closeStream() ;
}

void Engine::startMidi(Engine* engine){
    RtMidiIn* midiIn = engine->getMidiIn() ;
    MidiController* midiController = engine->getMidiController() ;
    int deviceId = engine->getMidiDeviceId() ;

    midiIn->openPort(deviceId);
    midiIn->setCallback(&MidiController::onMidiEvent, static_cast<void*>(midiController) );
    midiIn->ignoreTypes(false, false, false); // Do not ignore any MIDI message types

    std::cout << "Listening for MIDI input... (Ctrl+C to quit)\n";
    while (!stop_flag) {
        std::this_thread::sleep_for(std::chrono::seconds(1)); // Keep app alive
    }
}

void Engine::stopMidi(){
    if ( midiIn_.isPortOpen() ) midiIn_.closePort() ;
}

// Constructors
Engine::Engine():
    componentManager(&midiController),
    componentFactory(&componentManager),
    signalController(&componentManager), 
    midiController(&midiState_),
    dac_(),
    availableAudioDevices_(),
    selectedAudioOutput_(0),
    midiIn_(),
    availableMidiPorts_(),
    selectedMidiPort_(-1),
    midiState_(),
    midiDefaultHandler_()
{
    registerBaseMidiHandler(&midiDefaultHandler_);
    midiController.addHandler(&midiDefaultHandler_);

    signal(SIGINT, Engine::signalHandler);
}

// state functions
void Engine::initialize(){
    Config::load() ;

    // get midi list
    int numPorts = midiIn_.getPortCount() ;
    for (int i = 0 ; i<numPorts ; ++i ){
        availableMidiPorts_[i] = midiIn_.getPortName(i) ;
    }

    // get audio device list
    std::vector<unsigned int> devices = dac_.getDeviceIds() ;
    for ( auto dev : devices){
        auto info = dac_.getDeviceInfo(dev);
        availableAudioDevices_[info.ID] = info.name ;
    }

    // initialize server thread
    std::thread(std::bind(ApiHandler::start, this)).detach();

    while (!stop_flag) {
        std::this_thread::sleep_for(std::chrono::seconds(1)); // Keep app alive
    }
    
}

void Engine::run(){
    setup() ;    

    // start up the midi and audio threads
    std::thread t1(std::bind(Engine::startMidi, this));
    std::thread t2(std::bind(Engine::startAudio, this));

    startAnalysis();

    // wait for threads to finish
    t1.detach() ;
    t2.detach() ;
}

void Engine::stop(){
    stopAudio();
    stopMidi();
    destroy();
}

RtAudio* Engine::getDac(){
    return &dac_ ;
}

RtMidiIn* Engine::getMidiIn(){
    return &midiIn_ ;
}

MidiController* Engine::getMidiController(){
    return &midiController ;
}

MidiEventHandler* Engine::getDefaultMidiHandler(){
    return &midiDefaultHandler_ ;
}

double Engine::getDeltaTime() const {
    return dt_ ;
}
int Engine::getAudioDeviceId() const {
    return selectedAudioOutput_ ;
}

void Engine::setAudioDeviceId(int deviceId){
    std::cout << "audio output device set to id " << deviceId << "." << std::endl ;
    selectedAudioOutput_ = deviceId ;
}

int Engine::getMidiDeviceId() const {
    return selectedMidiPort_ ;
}

void Engine::setMidiDeviceId(int deviceId){
    std::cout << "midi device set to id " << deviceId << "." << std::endl ;
    selectedMidiPort_ = deviceId ;
}

const std::map<int,std::string> Engine::getAvailableMidiDevices() const {
    return availableMidiPorts_ ;
}

const std::map<int, std::string> Engine::getAvailableAudioDevices() const {
    return availableAudioDevices_ ;
}

// Set up Modules
void Engine::setup(){
    unsigned int buffer = Config::get<unsigned int>("audio.buffer_size").value() ;
    unsigned int sampleRate = Config::get<unsigned int>("audio.sample_rate").value() ;
    dt_ = 1.0 / sampleRate ;

    midiController.initialize() ;

    signalController.setup();

}

void Engine::destroy(){
    componentManager.reset();
    signalController.reset();
    midiState_.reset();
}

bool Engine::setMidiConnection(MidiEventHandler* outputMidi, MidiEventListener* listener){
    // if inputMidi is not a valid handler, use the default handler
    if ( !outputMidi ){
        std::cerr << "WARN: specified midi handler is not a valid object. Unable to successfully set a midi connection" << std::endl ;
        return false ;
    }

    if ( !listener ){
        std::cerr << "WARN: listener is a null pointer. Unable to successfully set a midi connection" << std::endl ;
        return false ;
    } 

    outputMidi->addListener(listener);
    return true ;
}

bool Engine::removeMidiConnection(MidiEventHandler* outputMidi, MidiEventListener* listener){
    if ( !outputMidi ){
        std::cerr << "WARN: specified midi handler is not a valid object. Unable to successfully remove a midi connection" << std::endl ;
        return false ;
    }

    if ( !listener ){
        std::cerr << "WARN: listener is a null pointer. Unable to successfully remove a midi connection" << std::endl ;
        return false ;
    } 

    outputMidi->removeListener(listener);    
    return true ;
}

bool Engine::registerBaseMidiHandler(MidiEventHandler* handler){
    if ( !handler ){
        std::cerr << "WARN: handler is a null pointer. Unable to register." << std::endl ;
        return false ;
    }
    midiState_.addHandler(handler);
    return true ;
}

bool Engine::unregisterBaseMidiHandler(MidiEventHandler* handler){
    if ( !handler ){
        std::cerr << "WARN: handler is a null pointer. Unable to register." << std::endl ;
        return false ;
    }
    midiState_.removeHandler(handler);
    return true ;
}

void Engine::startAnalysis(){
    std::thread([this](){
        std::vector<double> buffer(4096);

        while (!stop_flag){
            size_t count = analysisAudioOut_.pop(buffer.data(), buffer.size());

            if ( count > 0 ){
                analyzeBuffer(buffer.data(), count);
            }

            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
    }).detach();
}

void Engine::analyzeBuffer(double* data, size_t count){
    float lastSample = 0.0f ;
    std::atomic<uint64_t> numClicks{0};
    std::atomic<float> clickDiffSum{0};
    float diffThreshold = 0.3f ;

    // std::cout << "samples: " ;
    // for ( size_t i = 0; i < count; ++i ){
        // float diff = std::abs(data[i] - lastSample);
        // std::cout << data[i] << " " ;

        // if ( diff > diffThreshold ){
        //     numClicks++;
        //     clickDiffSum+=diff ;
        //     lastSample = data[i];
        // }
    // }
    // std::cout << std::endl ;


    // if (numClicks > 0)
    //     std::cout << numClicks << " clicks detected in " 
    //         << count << " samples. Average click sample value difference: " 
    //         << clickDiffSum/numClicks << std::endl ;
}