#ifndef __HPP_CONFIGS_ALL_MODULES
#define __HPP_CONFIGS_ALL_MODULES

#include "types/ModuleType.hpp"
#include "configs/OscillatorConfig.hpp"
#include "configs/PolyOscillatorConfig.hpp"

#include <nlohmann/json.hpp>
using json = nlohmann::json ;


#define HANDLE_DEFAULT_CONFIG(Type) \
    case ModuleType::Type: \
        return  ModuleTypeTraits<ModuleType::Type>::config{};

namespace Module {
    inline json getDefaultConfig(ModuleType type){
        switch(type){
        HANDLE_DEFAULT_CONFIG(Oscillator)
        HANDLE_DEFAULT_CONFIG(PolyOscillator)
        default:
            return json::object();
        }
    }
}


#endif // __HPP_CONFIGS_ALL_MODULES