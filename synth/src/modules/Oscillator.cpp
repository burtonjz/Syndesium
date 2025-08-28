#include "modules/Oscillator.hpp"
#include "types/ModuleType.hpp"
#include "dsp/Wavetable.hpp"
#include "params/ParameterMap.hpp"
#include "types/ParameterType.hpp"
#include <cmath>
#include <cstdint>

Oscillator::Oscillator(double sample_rate, std::size_t buf_size, OscillatorConfig cfg):
    BaseModule(ModuleType::Oscillator, sample_rate, buf_size),
    phase_(0),
    increment_(0)
{
    Wavetable::generate();

    parameters_.add<ParameterType::WAVEFORM>(cfg.waveform,false);
    parameters_.add<ParameterType::AMPLITUDE>(1.0,true);
    parameters_.add<ParameterType::FREQUENCY>(cfg.frequency, true, 0.0, sample_rate / 2.0); // limit to nyquist frequency
    parameters_.add<ParameterType::GAIN>(1.0,false);
}

Oscillator::Oscillator(double sample_rate, std::size_t buf_size, ParameterMap& parent, double frequency):
    BaseModule(ModuleType::Oscillator, sample_rate, buf_size),
    phase_(0),
    increment_(0)
{
    Wavetable::generate();

    parameters_.addReferences(parent);
    parameters_.add<ParameterType::AMPLITUDE>(1.0,true);
    parameters_.add<ParameterType::FREQUENCY>(frequency, true, 0.0, sample_rate / 2.0); // limit to nyquist frequency
}

bool Oscillator::isGenerative() const {
    return true ;
}

void Oscillator::calculateSample(){
    Waveform wf = Waveform::from_uint8(parameters_.getValue<ParameterType::WAVEFORM>()); 
    auto w = Wavetable::getWavetable(wf) ;
    double frac, wavetableIndex, sample ;
    int index_floor ;

    // calculate sample
    wavetableIndex = phase_ * (w.second - 1) ;
    index_floor = static_cast<int>(std::floor(wavetableIndex));
    frac = wavetableIndex - index_floor ;
    sample = ( 1.0 - frac ) * w.first[index_floor] + frac * w.first[index_floor+1];
    sample *= parameters_.getInstantaneousValue<ParameterType::AMPLITUDE>() * parameters_.getInstantaneousValue<ParameterType::GAIN>();
    
    buffer_[bufferIndex_] = sample ;
}

void Oscillator::tick(){
    BaseModule::tick();
    increment_ = parameters_.getInstantaneousValue<ParameterType::FREQUENCY>() / sampleRate_ ;
    phase_ = std::fmod(phase_ + increment_, 1.0);
    parameters_.modulate();
}

void Oscillator::addReferenceParameters(ParameterMap& map){
    parameters_.addReferences(map);
}

void Oscillator::setWaveform(Waveform wave){
    parameters_.setValue<ParameterType::WAVEFORM>(wave) ;
}

void Oscillator::setFrequency(double freq){
    parameters_.setValue<ParameterType::FREQUENCY>(freq) ;
}

void Oscillator::setAmplitude(double amp){
    parameters_.setValue<ParameterType::AMPLITUDE>(amp);
}

