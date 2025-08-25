#include "params/Parameter.hpp"

void ParameterBase::initializeDepth(){
    if (modulatable_ && !modDepth_){
        modDepth_ = std::make_unique<Parameter<ParameterType::DEPTH>>(
            parameterDefaults[static_cast<float>(ParameterType::DEPTH)],
            true
        );
    }
}

Parameter<ParameterType::DEPTH>* ParameterBase::getDepth() {
    return modDepth_.get() ;
}