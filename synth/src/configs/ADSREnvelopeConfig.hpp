#ifndef __HPP_CONFIGS_ADSRENVELOPE_
#define __HPP_CONFIGS_ADSRENVELOPE_

#include "types/ModulatorType.hpp"
#include <nlohmann/json.hpp>

using json = nlohmann::json ;

// forward declare class
class ADSREnvelope ;

// define default configuration
struct ADSREnvelopeConfig {
    double attack = 0.1 ;
    double decay = 0.1 ;
    double sustain = 0.8 ;
    double release = 0.1 ; 
};

// Type Traits Specification
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(ADSREnvelopeConfig, attack, release) // macro to serialize/deserialize json <-> structs

template <> struct ModulatorTypeTraits<ModulatorType::ADSREnvelope>{ 
    using type = ADSREnvelope ;
    using config = ADSREnvelopeConfig ;
};

#endif // __HPP_CONFIGS_ADSRENVELOPE_