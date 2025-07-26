#ifndef __SHARED_MODULE_DESCRIPTOR_HPP_
#define __SHARED_MODULE_DESCRIPTOR_HPP_

#include <string>
#include <set>

#include "types/ParameterType.hpp"
#include "types/ModuleType.hpp"
#include "types/ModulatorType.hpp"

struct ModuleDescriptor {
    std::string name ;
    ModuleType type ;
    std::set<ParameterType> modulatableParameters ;
    std::set<ParameterType> controllableParameters ;

    size_t numAudioInputs  ;
    size_t numAudioOutputs ;
    size_t numMidiInputs ;
    size_t numMidiOutputs ;

    bool isPolyphonic = false ;
};

#endif // __SHARED_MODULE_DESCRIPTOR_HPP_