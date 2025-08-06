#ifndef __MODULE_TYPE_HPP_
#define __MODULE_TYPE_HPP_

#include "types/Waveform.hpp"
#include <nlohmann/json.hpp>

/**
 * @brief enumeration of modulation source classes
 * 
*/
enum class ModuleType {
    Oscillator,
    PolyOscillator,
    N_MODULES
};

constexpr int N_MODULE_TYPES = static_cast<int>(ModuleType::N_MODULES) ;


// Define ModuleTypeTraits, which will convert from enum to the actual type (defined in each Module header)
template <ModuleType Type> struct ModuleTypeTraits ;

// Define configuration structures for module-specific arguments
template <ModuleType Type> struct ModuleConfig ; // will specify the specific {Module}Config structure

struct PolyOscillatorConfig {
    Waveform waveform = Waveform::SINE ;
};
template <> struct ModuleConfig<ModuleType::PolyOscillator>{ using cfg = PolyOscillatorConfig ;};
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(PolyOscillatorConfig, waveform) // macro to serialize/deserialize json <-> structs

struct OscillatorConfig {
    Waveform waveform = Waveform::SINE ;
    double frequency = 440.0 ;
};
template <> struct ModuleConfig<ModuleType::Oscillator>{ using cfg = OscillatorConfig;};
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(OscillatorConfig, waveform, frequency) // macro to serialize/deserialize json <-> structs

#define HANDLE_DEFAULT_CONFIG(Type) \
    case ModuleType::Type: \
        return  ModuleConfig<ModuleType::Type>::cfg{};

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


#endif // __MODULE_TYPE_HPP_