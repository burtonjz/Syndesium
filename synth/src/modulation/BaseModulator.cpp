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