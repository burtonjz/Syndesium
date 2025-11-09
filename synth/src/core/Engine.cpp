/*
 * Engine.cpp - Refactored with proper thread management
 */

#include "core/Engine.hpp"
#include "config/Config.hpp"
#include "api/ApiHandler.hpp"
#include "midi/MidiEventHandler.hpp"
#include "midi/MidiEventListener.hpp"
#include "types/Waveform.hpp"
#include "midi/MidiController.hpp"

#include <chrono>
#include <csignal>
#include <iostream>
#include <netinet/in.h>
#include <sys/socket.h>
#include <fcntl.h>
#include <thread>
#include <unistd.h>

// Static members
std::atomic<bool> Engine::stop_flag{false};

void Engine::signalHandler(int signum){
    std::cout << "Caught signal " << signum << ", stopping threads...\n";
    stop_flag = true;
}

// Constructor
Engine::Engine():
    // publically available interfaces
    componentManager(&midiController),
    componentFactory(&componentManager),
    signalController(&componentManager), 
    midiController(&midiState_),
    // thread state flags
    apiServerRunning_(false),
    engineRunning_(false),
    midiRunning_(false),
    audioRunning_(false),
    analysisRunning_(false),
    // audio 
    dac_(),
    availableAudioDevices_(),
    selectedAudioOutput_(0),
    // midi
    midiIn_(),
    availableMidiPorts_(),
    selectedMidiPort_(-1),
    midiState_(),
    midiDefaultHandler_(),
    // analysis
    analysisAudioOut_(48000 * 10)
{
    registerBaseMidiHandler(&midiDefaultHandler_);
    midiController.addHandler(&midiDefaultHandler_);
    signal(SIGINT, Engine::signalHandler);
}

// Destructor - ensure clean shutdown
Engine::~Engine() {
    shutdown();
}

// ============================================================================
// STATE FUNCTIONS
// ============================================================================

void Engine::initialize(){
    Config::load();

    // Get MIDI list
    int numPorts = midiIn_.getPortCount();
    for (int i = 0; i < numPorts; ++i){
        availableMidiPorts_[i] = midiIn_.getPortName(i);
    }

    // Get audio device list
    std::vector<unsigned int> devices = dac_.getDeviceIds();
    for (auto dev : devices){
        auto info = dac_.getDeviceInfo(dev);
        availableAudioDevices_[info.ID] = info.name;
    }

    // Start API server thread
    apiServerRunning_ = true;
    apiServerThread_ = std::thread(ApiHandler::start, this);

    std::cout << "Engine initialized. API server running." << std::endl;

    while ( !stop_flag && apiServerRunning_ ){
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
}

void Engine::run(){
    std::lock_guard<std::mutex> lock(stateMutex_);
    
    if (engineRunning_){
        std::cout << "Engine already running!" << std::endl;
        return;
    }

    std::cout << "Starting engine..." << std::endl;
    
    setup();
    
    engineRunning_ = true;
    
    // start threads
    midiRunning_ = true;
    midiThread_ = std::thread(&Engine::midiLoop, this);
    
    audioRunning_ = true;
    audioThread_ = std::thread(&Engine::audioLoop, this);
    
    analysisRunning_ = true;
    analysisThread_ = std::thread(&Engine::analysisLoop, this);
    
    std::cout << "Engine running with 3 worker threads" << std::endl;
}

void Engine::stop(){
    std::unique_lock<std::mutex> lock(stateMutex_);
    
    if (!engineRunning_){
        std::cout << "Engine not running!" << std::endl;
        return;
    }
    
    std::cout << "Stopping engine..." << std::endl;
    
    // Signal all threads to stop
    engineRunning_ = false;
    audioRunning_ = false;
    midiRunning_ = false;
    analysisRunning_ = false;
    
    // Unlock before joining to avoid deadlock
    lock.unlock();
    
    stopAudio();
    
    if (audioThread_.joinable()){
        std::cout << "Waiting for audio thread..." << std::endl;
        audioThread_.join();
    }
    
    if (midiThread_.joinable()){
        std::cout << "Waiting for MIDI thread..." << std::endl;
        midiThread_.join();
    }
    
    if (analysisThread_.joinable()){
        std::cout << "Waiting for analysis thread..." << std::endl;
        analysisThread_.join();
    }
    
    stopMidi();
    
    std::cout << "Engine stopped" << std::endl;
}

void Engine::shutdown(){
    std::cout << "Shutting down engine..." << std::endl;
    
    // Stop engine if running
    if (engineRunning_){
        stop();
    }
    
    // Stop API server
    stop_flag = true;
    apiServerRunning_ = false;
    
    if (apiServerThread_.joinable()){
        std::cout << "Waiting for API server thread..." << std::endl;
        apiServerThread_.join();
    }
    
    std::cout << "Engine shutdown complete" << std::endl;
}

// ============================================================================
// THREAD LOOPS
// ============================================================================

void Engine::midiLoop(){
    std::cout << "MIDI thread started" << std::endl;
    
    // Open MIDI port
    int deviceId = getMidiDeviceId();
    if (deviceId >= 0){
        midiIn_.openPort(deviceId);
        midiIn_.setCallback(&MidiController::onMidiEvent, static_cast<void*>(&midiController));
        midiIn_.ignoreTypes(false, false, false);
        std::cout << "Listening for MIDI input on port " << deviceId << std::endl;
    }
    
    // Keep thread alive while running
    while (midiRunning_ && engineRunning_){
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
    
    std::cout << "MIDI thread stopping" << std::endl;
}

void Engine::audioLoop(){
    std::cout << "Audio thread started" << std::endl;
    
    // Initialize audio
    unsigned int buffer = Config::get<unsigned int>("audio.buffer_size").value();
    unsigned int sampleRate = Config::get<unsigned int>("audio.sample_rate").value();
    
    RtAudio::StreamParameters parameters;
    std::map<int,std::string> devices = getAvailableAudioDevices();
    int deviceId = getAudioDeviceId();
    RtAudio::DeviceInfo deviceInfo;
    void* userData = static_cast<void*>(this);
    
    // Select device
    if (deviceId == 0 || devices.find(deviceId) == devices.end()){
        deviceInfo = dac_.getDeviceInfo(dac_.getDefaultOutputDevice());
    } else {
        deviceInfo = dac_.getDeviceInfo(deviceId);
    }
    
    parameters.deviceId = deviceInfo.ID;
    parameters.nChannels = 1;
    parameters.firstChannel = 0;
    
    // Validate sample rate
    auto &sampleRates = deviceInfo.sampleRates;
    if (std::count(sampleRates.begin(), sampleRates.end(), sampleRate) == 0){
        std::cout << "Configured sample rate of " << sampleRate 
                  << " is not supported by device " << deviceInfo.name 
                  << ". Setting to device preferred sample rate of " 
                  << deviceInfo.preferredSampleRate << "." << std::endl;
        sampleRate = deviceInfo.preferredSampleRate;
        Config::set("audio.sample_rate", sampleRate);
    }

    dt_ = 1.0 / sampleRate ;
    
    // Open audio stream
    if (dac_.openStream(
        &parameters,
        NULL,
        RTAUDIO_FLOAT64,
        sampleRate,
        &buffer,
        &audioCallback,
        userData
    )){
        std::cerr << "Error initializing audio: " << dac_.getErrorText() << std::endl;
        audioRunning_ = false;
        return;
    }
    
    // Start the stream
    if (dac_.startStream()){
        std::cerr << "Error starting audio stream: " << dac_.getErrorText() << std::endl;
        if (dac_.isStreamOpen()){
            dac_.closeStream();
        }
        audioRunning_ = false;
        return;
    }
    
    std::cout << "Audio stream started" << std::endl;
    
    // Keep thread alive while running
    // The actual audio processing happens in the callback
    while (audioRunning_ && engineRunning_){
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
    
    std::cout << "Audio thread stopping" << std::endl;
}

void Engine::analysisLoop(){
    std::cout << "Analysis thread started" << std::endl;
    
    std::vector<double> buffer(4096);
    
    while (analysisRunning_ && engineRunning_){
        size_t count = analysisAudioOut_.pop(buffer.data(), buffer.size());
        
        if (count > 0){
            analyzeBuffer(buffer.data(), count);
        }
        
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
    
    std::cout << "Analysis thread stopping" << std::endl;
}

// ============================================================================
// AUDIO CALLBACK (runs in audio thread context)
// ============================================================================

int Engine::audioCallback(
    void *outputBuffer, [[maybe_unused]] void *inputBuffer, 
    unsigned int nBufferFrames, [[maybe_unused]] double streamTime, 
    [[maybe_unused]] RtAudioStreamStatus status, void *userData 
){
    Engine* engine = static_cast<Engine*>(userData);
    double* buffer = static_cast<double*>(outputBuffer);
    
    // Check if we should still be running
    if (!engine->audioRunning_ || !engine->engineRunning_){
        // Fill buffer with silence and signal to stop
        std::fill_n(buffer, nBufferFrames, 0.0);
        return 1; // Non-zero signals stream should stop
    }
    
    engine->signalController.clearBuffer();
    
    double sample;
    for (unsigned int i = 0; i < nBufferFrames; ++i){
        engine->midiController.tick(engine->getDeltaTime());
        engine->componentManager.runParameterModulation();
        sample = engine->signalController.processFrame();
        buffer[i] = sample;
    }
    
    engine->analysisAudioOut_.push(buffer, nBufferFrames);
    return 0;
}

// ============================================================================
// CLEANUP FUNCTIONS
// ============================================================================

void Engine::stopAudio(){
    std::cout << "Stopping audio stream..." << std::endl;
    if (dac_.isStreamRunning()){
        dac_.stopStream();
    }
    if (dac_.isStreamOpen()){
        dac_.closeStream();
    }
}

void Engine::stopMidi(){
    std::cout << "Stopping MIDI..." << std::endl;
    if (midiIn_.isPortOpen()){
        midiIn_.cancelCallback();
        midiIn_.closePort();
    }
}

void Engine::audioCleanup(RtAudio* dac, RtAudioErrorType error){
    if (error) std::cout << '\n' << dac->getErrorText() << '\n' << std::endl;
    if (dac->isStreamOpen()){
        dac->closeStream();
    }
}

// ============================================================================
// SETUP AND TEARDOWN
// ============================================================================

void Engine::setup(){
    unsigned int sampleRate = Config::get<unsigned int>("audio.sample_rate").value();
    dt_ = 1.0 / sampleRate;
    
    midiController.initialize();
    signalController.updateProcessingGraph();
}

void Engine::destroy(){
    componentManager.reset();
    signalController.reset();
    midiState_.reset();
}

// ============================================================================
// GETTERS AND SETTERS
// ============================================================================

RtAudio* Engine::getDac(){
    return &dac_;
}

RtMidiIn* Engine::getMidiIn(){
    return &midiIn_;
}

MidiController* Engine::getMidiController(){
    return &midiController;
}

MidiEventHandler* Engine::getDefaultMidiHandler(){
    return &midiDefaultHandler_;
}

double Engine::getDeltaTime() const {
    return dt_;
}

int Engine::getAudioDeviceId() const {
    return selectedAudioOutput_;
}

void Engine::setAudioDeviceId(int deviceId){
    std::cout << "Audio output device set to id " << deviceId << "." << std::endl;
    selectedAudioOutput_ = deviceId;
}

int Engine::getMidiDeviceId() const {
    return selectedMidiPort_;
}

void Engine::setMidiDeviceId(int deviceId){
    std::cout << "MIDI device set to id " << deviceId << "." << std::endl;
    selectedMidiPort_ = deviceId;
}

const std::map<int,std::string> Engine::getAvailableMidiDevices() const {
    return availableMidiPorts_;
}

const std::map<int, std::string> Engine::getAvailableAudioDevices() const {
    return availableAudioDevices_;
}

// ============================================================================
// MIDI CONNECTION MANAGEMENT
// ============================================================================

bool Engine::setMidiConnection(MidiEventHandler* outputMidi, MidiEventListener* listener){
    if (!outputMidi){
        std::cerr << "WARN: specified midi handler is not a valid object. "
                  << "Unable to successfully set a midi connection" << std::endl;
        return false;
    }

    if (!listener){
        std::cerr << "WARN: listener is a null pointer. "
                  << "Unable to successfully set a midi connection" << std::endl;
        return false;
    }

    outputMidi->addListener(listener);
    return true;
}

bool Engine::removeMidiConnection(MidiEventHandler* outputMidi, MidiEventListener* listener){
    if (!outputMidi){
        std::cerr << "WARN: specified midi handler is not a valid object. "
                  << "Unable to successfully remove a midi connection" << std::endl;
        return false;
    }

    if (!listener){
        std::cerr << "WARN: listener is a null pointer. "
                  << "Unable to successfully remove a midi connection" << std::endl;
        return false;
    }

    outputMidi->removeListener(listener);    
    return true;
}

bool Engine::registerBaseMidiHandler(MidiEventHandler* handler){
    if (!handler){
        std::cerr << "WARN: handler is a null pointer. Unable to register." << std::endl;
        return false;
    }
    midiState_.addHandler(handler);
    return true;
}

bool Engine::unregisterBaseMidiHandler(MidiEventHandler* handler){
    if (!handler){
        std::cerr << "WARN: handler is a null pointer. Unable to unregister." << std::endl;
        return false;
    }
    midiState_.removeHandler(handler);
    return true;
}

// ============================================================================
// ANALYSIS
// ============================================================================

void Engine::analyzeBuffer([[maybe_unused]] double* data, [[maybe_unused]] size_t count){
    // Your analysis implementation here
}