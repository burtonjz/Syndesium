#include "params/Parameter.hpp"

void ParameterBase::initializeDepth(){
    // depth will recurse until MAX_DEPTH, determined by it being identified as a modulatable parameter
    if (!modulatable_ && !modDepth_) return ;
    if ( modDepthLevel_ >= MAX_DEPTH ) return ;

    bool nextModulatable = (modDepthLevel_ + 1) < MAX_DEPTH ;

    modDepth_ = std::make_unique<Parameter<ParameterType::DEPTH>>(
        parameterDefaults[static_cast<float>(ParameterType::DEPTH)],
        nextModulatable,
        modDepthLevel_ + 1
    );
    
}

Parameter<ParameterType::DEPTH>* ParameterBase::getDepth() {
    initializeDepth();
    return modDepth_.get() ;
}