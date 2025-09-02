#include "params/Parameter.hpp"
#include "config/Config.hpp"

void ParameterBase::initializeDepth(){
    if (!modulatable_ && !modDepth_) return ;
    
    int maxDepth = Config::get<int>("modulation.max_depth").value() ;
    if ( modDepthLevel_ >= maxDepth ) return ;

    bool nextModulatable = (modDepthLevel_ + 1) < maxDepth ; // only modulate depth so far!

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