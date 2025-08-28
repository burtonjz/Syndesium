#ifndef __HPP_CONFIGS_ALL_MODULATORS
#define __HPP_CONFIGS_ALL_MODULATORS

#include "types/ModulatorType.hpp"
#include "configs/LinearFaderConfig.hpp"
#include "configs/ADSREnvelopeConfig.hpp"

#include <nlohmann/json.hpp>
using json = nlohmann::json ;


#define HANDLE_DEFAULT_CONFIG(Type) \
    case ModulatorType::Type: \
        return  ModulatorTypeTraits<ModulatorType::Type>::config{};

namespace Modulator {
    inline json getDefaultConfig(ModulatorType type){
        switch(type){
        HANDLE_DEFAULT_CONFIG(LinearFader)
        HANDLE_DEFAULT_CONFIG(ADSREnvelope)
        default:
            return json::object();
        }
    }
}


#endif // __HPP_CONFIGS_ALL_MODULATORS