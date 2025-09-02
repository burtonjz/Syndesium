#include "modulation/BaseModulator.hpp"
#include "params/ParameterMap.hpp"

BaseModulator::BaseModulator(ModulatorType typ):
        type_(typ),
        parameters_(std::make_unique<ParameterMap>())
    {}

ModulatorType BaseModulator::getType() const {
    return type_ ;
}

ParameterMap* BaseModulator::getParameters(){
    return parameters_.get() ;
}

void BaseModulator::setParameterModulation(ParameterType p, BaseModulator* m, ModulationData d ){
    if ( ! parameters_ ) return ;
    if ( d.empty() ){
        auto required = m->getRequiredModulationParameters();
        for ( auto mp : required ){
            d[mp];
        }
    }
    parameters_->setModulation(p,m,d);
}

void BaseModulator::tick(){
    if (parameters_) parameters_->modulate() ;
}