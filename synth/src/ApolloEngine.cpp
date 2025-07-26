#include "ApolloEngine.hpp"
#include "config/Config.hpp"
#include "api/ApiHandler.hpp"
#include "midi/MidiEventHandler.hpp"
#include "midi/MidiEventQueue.hpp"
#include "types/ModulatorType.hpp"
#include "types/ModuleType.hpp"
#include "types/Waveform.hpp"
#include "dsp/math.hpp"
#include "midi/MidiController.hpp"
#include "types/ParameterType.hpp"

#include "modules/PolyOscillator.hpp"

#include <chrono>
#include <csignal>
#include <iostream>
#include <netinet/in.h>
#include <sys/socket.h>
#include <fcntl.h>
#include <thread>
#include <unistd.h>


// static members and functions
std::atomic<bool> ApolloEngine::stop_flag{false};

void ApolloEngine::signalHandler(int signum){
    std::cout << "Caught signal " << signum << ", stopping threads...\n" ;
    stop_flag = true ;
}

void ApolloEngine::startAudio(ApolloEngine* engine){
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

void ApolloEngine::stopAudio(){
    if ( dac_.isStreamRunning() ) dac_.stopStream();
    if ( dac_.isStreamOpen() ) dac_.closeStream();
}

int ApolloEngine::audioCallback(
    void *outputBuffer, void *inputBuffer, unsigned int nBufferFrames, double streamTime, 
    RtAudioStreamStatus status, void *userData 
){
    ApolloEngine* engine = static_cast<ApolloEngine*>(userData);
    double* buffer = static_cast<double*>(outputBuffer) ;
    
    engine->moduleController.clearBuffer();

    double sample ;
    for ( unsigned int i = 0 ; i < nBufferFrames ; ++i ){
        sample = engine->moduleController.processFrame();
        engine->modulationController.tick(engine->getDeltaTime());
        buffer[i] = dsp::fastAtan(sample) ;
        std::cout << buffer[i] << "\n" ;
    }
    
    return 0 ;
}

void ApolloEngine::audioCleanup(RtAudio* dac, RtAudioErrorType error){
    if (error) std::cout << '\n' << dac->getErrorText() << '\n' << std::endl ;
    if (dac->isStreamOpen()) dac->closeStream() ;
}

void ApolloEngine::startMidi(ApolloEngine* engine){
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

void ApolloEngine::stopMidi(){
    if ( midiIn_.isPortOpen() ) midiIn_.closePort() ;
}

// Constructors
ApolloEngine::ApolloEngine():
    dac_(),
    availableAudioDevices_(),
    selectedAudioOutput_(0), // invalid device ID
    midiIn_(),
    availableMidiPorts_(),
    selectedMidiPort_(-1), // invalid device ID
    midiState_(),
    midiController_(&midiState_),
    moduleController(),
    modulationController()
{
    signal(SIGINT, ApolloEngine::signalHandler);
}

// state functions
void ApolloEngine::initialize(){
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

void ApolloEngine::run(){
    setup() ;    

    // start up the midi and audio threads
    std::thread t1(std::bind(ApolloEngine::startMidi, this));
    std::thread t2(std::bind(ApolloEngine::startAudio, this));

    // wait for threads to finish
    t1.detach() ;
    t2.detach() ;
}

void ApolloEngine::stop(){
    stopAudio();
    stopMidi();
    destroy();
}

RtAudio* ApolloEngine::getDac(){
    return &dac_ ;
}

RtMidiIn* ApolloEngine::getMidiIn(){
    return &midiIn_ ;
}

MidiController* ApolloEngine::getMidiController(){
    return &midiController_ ;
}

double ApolloEngine::getDeltaTime() const {
    return dt_ ;
}
int ApolloEngine::getAudioDeviceId() const {
    return selectedAudioOutput_ ;
}

void ApolloEngine::setAudioDeviceId(int deviceId){
    std::cout << "audio output device set to id " << deviceId << "." << std::endl ;
    selectedAudioOutput_ = deviceId ;
}

int ApolloEngine::getMidiDeviceId() const {
    return selectedMidiPort_ ;
}

void ApolloEngine::setMidiDeviceId(int deviceId){
    std::cout << "midi device set to id " << deviceId << "." << std::endl ;
    selectedMidiPort_ = deviceId ;
}

const std::map<int,std::string> ApolloEngine::getAvailableMidiDevices() const {
    return availableMidiPorts_ ;
}

const std::map<int, std::string> ApolloEngine::getAvailableAudioDevices() const {
    return availableAudioDevices_ ;
}

// Set up Modules
void ApolloEngine::setup(){
    unsigned int buffer = Config::get<unsigned int>("audio.buffer_size").value() ;
    unsigned int sampleRate = Config::get<unsigned int>("audio.sample_rate").value() ;
    dt_ = 1.0 / sampleRate ;

    midiState_.reset();
    midiController_.initialize() ;

    // TEMPORARY MANUAL SIGNAL CHAIN CREATION
    // make the polyphonic oscillator
    std::cout << "creating polyphonic oscillator..." << std::endl ;
    auto m = moduleController.create<ModuleType::PolyOscillator>("Oscillator1", sampleRate, buffer, Waveform::SINE);
    auto osc = moduleController.get<ModuleType::PolyOscillator>(m);
    std::cout <<  "oscillator created with address " << osc << std::endl ;

    moduleController.registerSink(moduleController.getRaw(m));
    
    // create a fader
    auto f = modulationController.create<ModulatorType::LinearFader>("fader1", &midiState_);
    auto fader = modulationController.get<LinearFader>(f);
    
    // register the oscillator to the fader
    auto h = static_cast<MidiEventHandler*>(fader);
    midiState_.addHandler(h);
    h->addListener(osc);
    ModulationData d ;
    d[ModulationParameter::MIDI_NOTE];
    d[ModulationParameter::INITIAL_VALUE];
    d[ModulationParameter::LAST_VALUE];
    osc->setModulation(ParameterType::AMPLITUDE, fader, d);

    moduleController.setup();

}

void ApolloEngine::destroy(){
    modulationController.reset();
    moduleController.reset();
    midiState_.reset();
}