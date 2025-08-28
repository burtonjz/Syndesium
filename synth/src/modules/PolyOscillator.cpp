#include "modules/PolyOscillator.hpp"
#include "types/ModuleType.hpp"
#include "types/Waveform.hpp"
#include "midi/MidiEventHandler.hpp"
#include "params/ParameterMap.hpp"
#include "params/ModulationParameter.hpp"
#include "types/ParameterType.hpp"
#include "modulation/Modulator.hpp"
#include "config/Config.hpp"

#include <cmath>
#include <utility>
#include <iostream>


PolyOscillator::PolyOscillator(double sample_rate, std::size_t buf_size, PolyOscillatorConfig cfg):
    BaseModule(ModuleType::PolyOscillator, sample_rate, buf_size),
    MidiEventListener(),
    children_(),
    modulators_(),
    modulationData_()
{
    parameters_.add<ParameterType::WAVEFORM>(cfg.waveform,false);
    parameters_.add<ParameterType::GAIN>(1.0 , true);
    updateGain();

    childPool_.initializeAll(sampleRate_, size_, parameters_, 0.0);
}

ParameterMap* PolyOscillator::getParameters(){
    return &parameters_ ;
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
        for ( auto p : osc->getParameters()->getModulatableParameters()){
            if ( modulators_.find(p) != modulators_.end() ){
                auto d = osc->getParameters()->getModulationData(p);
                if ( 
                    d->find(ModulationParameter::INITIAL_VALUE) != modulationData_[p].end() &&
                    d->find(ModulationParameter::LAST_VALUE)    != modulationData_[p].end()
                ){
                    (*d)[ModulationParameter::INITIAL_VALUE].set((*d)[ModulationParameter::LAST_VALUE].get()) ;
                }
            }
        } 
        return ;
    }

    if ( Oscillator* osc = childPool_.allocate() ){
        osc->setBufferIndex(bufferIndex_); // sync up buffers
        osc->addReferenceParameters(parameters_);
        osc->setFrequency(anote->note.getFrequency());
        osc->setAmplitude(anote->note.getMidiVelocity() / 127.0 );
        
        // handle modulation updates
        for ( auto p : osc->getParameters()->getModulatableParameters()){
            if ( modulators_.find(p) != modulators_.end() ){
                if ( modulationData_[p].find(ModulationParameter::MIDI_NOTE) != modulationData_[p].end() ){
                    modulationData_[p][ModulationParameter::MIDI_NOTE].set(anote->note.getMidiNote()) ;
                }
                if ( modulationData_[p].find(ModulationParameter::INITIAL_VALUE) != modulationData_[p].end() ){
                    modulationData_[p][ModulationParameter::INITIAL_VALUE].set(0.0f) ;
                }
                osc->getParameters()->setModulation(p, modulators_[p], modulationData_[p]);
            }
        }
        osc->getParameters()->modulate() ; // prime modulation to not click on first sample
        children_.insert(std::make_pair(anote->note.getMidiNote(),osc));
    } 
}

void PolyOscillator::onKeyReleased(ActiveNote anote){
    auto it = children_.find(anote.note.getMidiNote());
    if ( it != children_.end() ){
        for ( auto p : it->second->getParameters()->getModulatableParameters()){
            if ( modulators_.find(p) != modulators_.end() ){
                auto d = it->second->getParameters()->getModulationData(p);
                if ( 
                    d->find(ModulationParameter::INITIAL_VALUE) != modulationData_[p].end() &&
                    d->find(ModulationParameter::LAST_VALUE)    != modulationData_[p].end()
                ){
                    (*d)[ModulationParameter::INITIAL_VALUE].set((*d)[ModulationParameter::LAST_VALUE].get()) ;
                }
            }
        }
    // set modulation
    }
}

void PolyOscillator::onKeyOff(ActiveNote anote){
    auto it = children_.find(anote.note.getMidiNote());
    if ( it != children_.end() ){
        childPool_.release(it->second);
        children_.erase(it);
    }
}

void PolyOscillator::setModulation(ParameterType p, Modulator* m, ModulationData d){
    modulators_[p] = m ;
    modulationData_[p] = d ;
    MidiEventHandler* h = dynamic_cast<MidiEventHandler*>(m);
    if (h) h->addListener(this);
}

void PolyOscillator::updateGain(){
    auto s = std::string(Waveform::getWaveforms()[parameters_.getValue<ParameterType::WAVEFORM>()]) ;
    float gain = Config::get<float>("oscillator." + s + ".auto_gain").value() / 
        std::sqrt(Config::get<int>("oscillator.expected_voices").value()) ;
    std::cout << "setting gain to " << gain << std::endl ;
    parameters_.setValue<ParameterType::GAIN>(gain);
}