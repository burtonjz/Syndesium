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

#include "components/PolyOscillator.hpp"
#include "core/BaseComponent.hpp"
#include "types/ComponentType.hpp"
#include "types/Waveform.hpp"
#include "params/ParameterMap.hpp"
#include "params/ModulationParameter.hpp"
#include "types/ParameterType.hpp"
#include "core/BaseModulator.hpp"
#include "config/Config.hpp"

#include <cmath>
#include <utility>
#include <iostream>


PolyOscillator::PolyOscillator(ComponentId id, PolyOscillatorConfig cfg):
    BaseComponent(id, ComponentType::PolyOscillator),
    BaseModule(),
    MidiEventListener(),
    children_(),
    modulators_{},
    modulationData_({})
{
    parameters_->add<ParameterType::WAVEFORM>(cfg.waveform,false);
    parameters_->add<ParameterType::GAIN>(1.0 , false);
    parameters_->add<ParameterType::DETUNE>(0, false);

    updateGain();

    childPool_.initializeAll(*parameters_, 0.0);
}

bool PolyOscillator::isGenerative() const {
    return true ;
}

void PolyOscillator::calculateSample(){
    childPool_.forEachActive([&](Oscillator& obj, std::size_t index){
        obj.calculateSample();
        buffer_[index] += obj.data()[index];
    }, bufferIndex_);
}

void PolyOscillator::tick(){
    BaseModule::tick();
    childPool_.forEachActive([&](Oscillator& obj){
        obj.tick();
    });
}

void PolyOscillator::clearBuffer(){
    std::fill(buffer_.get(), buffer_.get() + size_, 0.0);
    // now clear children
    childPool_.forEachActive([](Oscillator& obj){
        obj.clearBuffer();
    });
}

void PolyOscillator::onKeyPressed(const ActiveNote* anote, bool rePress){
    if ( rePress && children_.find(anote->note.getMidiNote())){
        Oscillator* osc = children_[anote->note.getMidiNote()] ;
        if ( osc ){
            osc->setFrequency(anote->note.getFrequency());
            osc->setAmplitude(anote->note.getMidiVelocity() / 127.0 );
            updateModulationInitialValue(osc);
        }
        return ;
    }

    if ( Oscillator* osc = childPool_.allocate() ){
        osc->setBufferIndex(bufferIndex_); // sync up buffers
        osc->addReferenceParameters(*parameters_);
        osc->setFrequency(anote->note.getFrequency());
        osc->setAmplitude(anote->note.getMidiVelocity() / 127.0 );
        
        // if there are modulators that need MIDI_NOTE, the module needs to make sure that gets set
        for ( auto p : osc->getParameters()->getModulatableParameters()){
            auto pIndex = static_cast<size_t>(p);
            if ( modulators_[pIndex] ){
                auto modParams = modulators_[pIndex]->getRequiredModulationParameters();
                if ( modParams.contains(ModulationParameter::MIDI_NOTE)){
                    modulationData_[static_cast<size_t>(p)].set(ModulationParameter::MIDI_NOTE, anote->note.getMidiNote());
                }
                osc->getParameters()->getParameter(p)->setModulation(modulators_[pIndex], modulationData_[pIndex]);
            }
        }
        children_.insert(std::make_pair(anote->note.getMidiNote(),osc));
    } 
}

void PolyOscillator::onKeyReleased(ActiveNote anote){
    auto it = children_.find(anote.note.getMidiNote());
    if ( it != children_.end() ){
        updateModulationInitialValue(it->second);
    }
}

void PolyOscillator::onKeyOff(ActiveNote anote){
    auto it = children_.find(anote.note.getMidiNote());
    if ( it != children_.end() ){
        childPool_.release(it->second);
        children_.erase(it);
    }
}

void PolyOscillator::updateParameters(){
    BaseComponent::updateParameters();
    childPool_.forEachActive(&Oscillator::updateParameters);
}

BaseModulator* PolyOscillator::getParameterModulator(ParameterType p) const {
    return modulators_[static_cast<size_t>(p)] ;
}

void PolyOscillator::onSetParameterModulation(ParameterType p, BaseModulator* m, ModulationData d){
    if ( d.isEmpty() ){
        auto required = m->getRequiredModulationParameters();
        for ( auto mp : required ){
            d.set(mp, 0.0f);
        }
    }
    modulators_[static_cast<size_t>(p)] = m ;
    modulationData_[static_cast<size_t>(p)] = d ;

    childPool_.forEachActive(&Oscillator::setParameterModulation, p, m, d);
}

void PolyOscillator::onRemoveParameterModulation(ParameterType p){
    modulators_[static_cast<size_t>(p)] = nullptr ;
    modulationData_[static_cast<size_t>(p)] = {} ;

    childPool_.forEachActive(&Oscillator::removeParameterModulation, p);
}

void PolyOscillator::updateGain(){
    auto s = std::string(Waveform::getWaveforms()[parameters_->getParameter<ParameterType::WAVEFORM>()->getValue()]) ;
    float gain = Config::get<float>("oscillator." + s + ".auto_gain").value() / 
        std::sqrt(Config::get<int>("oscillator.expected_voices").value()) ;
    std::cout << "setting gain to " << gain << std::endl ;
    parameters_->getParameter<ParameterType::GAIN>()->setValue(gain);
}

void PolyOscillator::updateModulationInitialValue(Oscillator* osc){
    for ( auto p : osc->getParameters()->getModulatableParameters()){
        if ( modulators_[static_cast<size_t>(p)] ){
            auto d = osc->getParameters()->getParameter(p)->getModulationData();
            if ( 
                d->has(ModulationParameter::INITIAL_VALUE) &&
                d->has(ModulationParameter::OUTPUT_1)
            ){
                d->set(ModulationParameter::INITIAL_VALUE, d->get(ModulationParameter::OUTPUT_1)) ;
            }
        }
    } 
}