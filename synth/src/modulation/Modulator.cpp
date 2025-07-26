#include "modulation/Modulator.hpp"
#include "params/ParameterMap.hpp"

Modulator::Modulator(ModulatorType typ):
        type_(typ),
        parameters_(std::make_unique<ParameterMap>())
    {}

ModulatorType Modulator::getType() const {
    return type_ ;
}

ParameterMap* Modulator::getParameters(){
    return parameters_.get() ;
}