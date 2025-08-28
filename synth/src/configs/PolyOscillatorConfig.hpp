#ifndef __HPP_CONFIGS_POLYOSCILLATOR_
#define __HPP_CONFIGS_POLYOSCILLATOR_

#include "types/ModuleType.hpp"
#include "types/Waveform.hpp"

#include <nlohmann/json.hpp>
using json = nlohmann::json ;

// forward declare class
class PolyOscillator ;

// define default configuration
struct PolyOscillatorConfig {
    Waveform waveform = Waveform::SINE ;
};

// Type Traits Specification
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(PolyOscillatorConfig, waveform) // macro to serialize/deserialize json <-> structs

template <> struct ModuleTypeTraits<ModuleType::PolyOscillator>{ 
    using type = PolyOscillator ;
    using config = PolyOscillatorConfig ;
};

#endif // __HPP_CONFIGS_POLYOSCILLATOR_