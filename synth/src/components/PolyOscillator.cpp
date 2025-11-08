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
    modulators_(),
    modulationData_()
{
    parameters_->add<ParameterType::WAVEFORM>(cfg.waveform,false);
    parameters_->add<ParameterType::GAIN>(1.0 , false);
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
        osc->setFrequency(anote->note.getFrequency());
        osc->setAmplitude(anote->note.getMidiVelocity() / 127.0 );
        updateModulationInitialValue(osc);
        return ;
    }

    if ( Oscillator* osc = childPool_.allocate() ){
        osc->setBufferIndex(bufferIndex_); // sync up buffers
        osc->addReferenceParameters(*parameters_);
        osc->setFrequency(anote->note.getFrequency());
        osc->setAmplitude(anote->note.getMidiVelocity() / 127.0 );
        
        // if there are modulators that need MIDI_NOTE, the module needs to make sure that gets set
        for ( auto p : osc->getParameters()->getModulatableParameters()){
            if ( modulators_.find(p) != modulators_.end() ){
                auto modParams = modulators_[p]->getRequiredModulationParameters();
                if ( modParams.contains(ModulationParameter::MIDI_NOTE)){
                    modulationData_[p][ModulationParameter::MIDI_NOTE].set(anote->note.getMidiNote());
                }
                osc->getParameters()->setModulation(p, modulators_[p], modulationData_[p]);
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

void PolyOscillator::onSetParameterModulation(ParameterType p, BaseModulator* m, ModulationData d){
    if ( d.empty() ){
        auto required = m->getRequiredModulationParameters();
        for ( auto mp : required ){
            d[mp];
        }
    }
    modulators_[p] = m ;
    modulationData_[p] = d ;

    childPool_.forEachActive(&Oscillator::setParameterModulation, p, m, d);
}

void PolyOscillator::onRemoveParameterModulation(ParameterType p){
    modulators_[p] = nullptr ;
    modulationData_[p] = {} ;

    childPool_.forEachActive(&Oscillator::removeParameterModulation, p);
}

void PolyOscillator::updateGain(){
    auto s = std::string(Waveform::getWaveforms()[parameters_->getValue<ParameterType::WAVEFORM>()]) ;
    float gain = Config::get<float>("oscillator." + s + ".auto_gain").value() / 
        std::sqrt(Config::get<int>("oscillator.expected_voices").value()) ;
    std::cout << "setting gain to " << gain << std::endl ;
    parameters_->setValue<ParameterType::GAIN>(gain);
}

void PolyOscillator::updateModulationInitialValue(Oscillator* osc){
    for ( auto p : osc->getParameters()->getModulatableParameters()){
        if ( modulators_.find(p) != modulators_.end() ){
            auto d = osc->getParameters()->getModulationData(p);
            if ( 
                d->find(ModulationParameter::INITIAL_VALUE) != modulationData_[p].end() &&
                d->find(ModulationParameter::OUTPUT_1)    != modulationData_[p].end()
            ){
                (*d)[ModulationParameter::INITIAL_VALUE].set((*d)[ModulationParameter::OUTPUT_1].get()) ;
            }
        }
    } 
}