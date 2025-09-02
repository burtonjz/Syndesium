#ifndef __HPP_CONFIGS_LINEARFADER_
#define __HPP_CONFIGS_LINEARFADER_

#include "types/ModulatorType.hpp"
#include <nlohmann/json.hpp>

using json = nlohmann::json ;

// forward declare class
class LinearFader ;

// define default configuration
struct LinearFaderConfig {
    double attack = 1.0 ;
    double release = 1.0 ; 
};

// Type Traits Specification
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(LinearFaderConfig, attack, release) // macro to serialize/deserialize json <-> structs

template <> struct ModulatorTypeTraits<ModulatorType::LinearFader>{ 
    using type = LinearFader ;
    using config = LinearFaderConfig ;
};

#endif // __HPP_CONFIGS_LINEARFADER_