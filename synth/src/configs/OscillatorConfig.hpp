#ifndef __HPP_CONFIGS_OSCILLATOR_
#define __HPP_CONFIGS_OSCILLATOR_

#include "types/ModuleType.hpp"
#include "types/Waveform.hpp"

// forward declare class
class Oscillator ;

// define default configuration
struct OscillatorConfig {
    Waveform waveform = Waveform::SINE ;
    double frequency = 440.0 ;
};

// Type Traits Specification
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(OscillatorConfig, waveform) // macro to serialize/deserialize json <-> structs

template <> struct ModuleTypeTraits<ModuleType::Oscillator>{ 
    using type = Oscillator ;
    using config = OscillatorConfig ;
};

#endif // __HPP_CONFIGS_OSCILLATOR_