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

#include "components/Oscillator.hpp"
#include "types/ComponentType.hpp"
#include "dsp/Wavetable.hpp"
#include "params/ParameterMap.hpp"
#include "types/ParameterType.hpp"
#include <cmath>
#include <cstdint>

Oscillator::Oscillator(ComponentId id, OscillatorConfig cfg):
    BaseComponent(id, ComponentType::Oscillator),
    BaseModule(),
    phase_(0),
    increment_(0)
{
    Wavetable::generate();

    parameters_->add<ParameterType::WAVEFORM>(cfg.waveform,false);
    parameters_->add<ParameterType::AMPLITUDE>(1.0,true);
    parameters_->add<ParameterType::FREQUENCY>(cfg.frequency, true, 0.0, sampleRate_ / 2.0); // limit to nyquist frequency
    parameters_->add<ParameterType::GAIN>(1.0,false);
}

Oscillator::Oscillator(ParameterMap& parent, double frequency):
    BaseComponent(-1, ComponentType::Oscillator),
    BaseModule(),
    phase_(0),
    increment_(0)
{
    Wavetable::generate();

    parameters_->addReferences(parent);
    parameters_->add<ParameterType::AMPLITUDE>(1.0,true);
    parameters_->add<ParameterType::FREQUENCY>(frequency, true, 0.0, sampleRate_ / 2.0); // limit to nyquist frequency
}

bool Oscillator::isGenerative() const {
    return true ;
}

void Oscillator::calculateSample(){
    Waveform wf = Waveform::from_uint8(parameters_->getValue<ParameterType::WAVEFORM>()); 
    auto w = Wavetable::getWavetable(wf) ;
    double frac, wavetableIndex, sample ;
    int index_floor ;

    // calculate sample
    wavetableIndex = phase_ * (w.second - 1) ;
    index_floor = static_cast<int>(std::floor(wavetableIndex));
    frac = wavetableIndex - index_floor ;
    sample = ( 1.0 - frac ) * w.first[index_floor] + frac * w.first[index_floor+1];
    sample *= parameters_->getInstantaneousValue<ParameterType::AMPLITUDE>() * parameters_->getInstantaneousValue<ParameterType::GAIN>();
    buffer_[bufferIndex_] = sample ;
}

void Oscillator::tick(){
    BaseModule::tick();
    increment_ = parameters_->getInstantaneousValue<ParameterType::FREQUENCY>() / sampleRate_ ;
    phase_ = std::fmod(phase_ + increment_, 1.0);
    parameters_->modulate();
}

void Oscillator::addReferenceParameters(ParameterMap& map){
    parameters_->addReferences(map);
}

void Oscillator::setWaveform(Waveform wave){
    parameters_->setValue<ParameterType::WAVEFORM>(wave) ;
}

void Oscillator::setFrequency(double freq){
    parameters_->setValue<ParameterType::FREQUENCY>(freq) ;
}

void Oscillator::setAmplitude(double amp){
    parameters_->setValue<ParameterType::AMPLITUDE>(amp);
}

