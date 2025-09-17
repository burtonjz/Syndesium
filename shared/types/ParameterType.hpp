#ifndef __PARAMETER_TYPE_HPP_
#define __PARAMETER_TYPE_HPP_

#include "types/ModulatorType.hpp"
#include "types/Waveform.hpp"

#include <array>
#include <utility>
#include <limits>
#include <cstdint>
#include <string_view>

/**
 * @brief types of parameters that might exist within any given module
 * 
*/
enum class ParameterType {
    DEPTH,
    STATUS,
    WAVEFORM,
    FREQUENCY,
    AMPLITUDE,
    GAIN,
    PHASE,
    PAN,
    DETUNE,
    ATTACK,
    DECAY,
    SUSTAIN,
    RELEASE,
    FILTER_TYPE,
    CUTOFF,
    Q_FACTOR,
    N_PARAMETERS
};

constexpr int N_PARAMETER_TYPES = static_cast<int>(ParameterType::N_PARAMETERS) ;

constexpr std::array<std::string_view, N_PARAMETER_TYPES> parameterStrings({
    "depth",
    "status",
    "waveform",
    "frequency",
    "amplitude",
    "gain",
    "phase",
    "pan",
    "detune",
    "attack",
    "decay",
    "sustain",
    "release",
    "filter type",
    "cutoff",
    "q factor"
});
const std::string parameter2String(ParameterType p);
ParameterType parameterFromString(std::string str);

/**
 * @brief a std::pair containing the min and max for each parameter type
*/
constexpr std::array<std::pair<float, float>, N_PARAMETER_TYPES> parameterLimits({
    std::make_pair(-5.0f, 5.0f),                             // DEPTH
    std::make_pair(0.0f, 1.0f),                              // STATUS
    std::make_pair(0.0f,static_cast<float>(Waveform::N)),    // WAVEFORM
    std::make_pair(0.0f, std::numeric_limits<float>::max()), // FREQUENCY
    std::make_pair(0.0f, 1.0f),                              // AMPLITUDE
    std::make_pair(0.0f, 1.0f),                              // GAIN
    std::make_pair(0.0f, 1.0f),                              // PHASE
    std::make_pair(-1.0f, 1.0f),                             // PAN
    std::make_pair(-1250.0f, 1250.0f),                       // DETUNE
    std::make_pair(0.001f, 4.0f ),                           // ATTACK
    std::make_pair(0.001f, 4.0f ),                           // DECAY
    std::make_pair(0.0f, 1.0f ),                            // SUSTAIN
    std::make_pair(0.001f, 4.0f ),                          // RELEASE
    std::make_pair(0.0f, 3.0f),                             // FILTER_TYPE  
    std::make_pair(0.0f, 20000.0f ),                        // CUTOFF
    std::make_pair(0.5f, 10.0f),                            // Q_FACTOR
});

/**
 * @brief a std::array containing the defaults for each parameter type
*/
constexpr std::array<float, N_PARAMETER_TYPES> parameterDefaults({
    1.0f,                               // DEPTH 
    0.0f,                               // STATUS
    static_cast<float>(Waveform::SINE), // WAVEFORM
    440.0f,                             // FREQUENCY
    1.0f,                               // AMPLITUDE
    1.0f,                               // GAIN
    0.0f,                               // PHASE
    0.0f,                               // PAN
    0.0f,                               // DETUNE
    0.01f,                              // ATTACK
    0.05f,                             // DECAY
    0.8f,                              // SUSTAIN
    0.2f,                              // RELEASE
    0.0f,                              // FILTER_TYPE
    0.0f,                              // CUTOFF
    0.5f                               // Q_FACTOR
});

/**
 * @brief define value variable type for each ParameterType
*/
template <ParameterType Type> struct ParameterTypeTraits ;
template <> struct ParameterTypeTraits<ParameterType::DEPTH>{
    using ValueType = float;
    static constexpr ModulationStrategy defaultStrategy = ModulationStrategy::NONE ;
};

template <> struct ParameterTypeTraits<ParameterType::STATUS>{
    using ValueType = bool;
    static constexpr ModulationStrategy defaultStrategy = ModulationStrategy::NONE ;
};

template <> struct ParameterTypeTraits<ParameterType::WAVEFORM>{
    using ValueType = uint8_t;
    static constexpr ModulationStrategy defaultStrategy = ModulationStrategy::NONE ;
};

template <> struct ParameterTypeTraits<ParameterType::FREQUENCY>{
    using ValueType = double;
    static constexpr ModulationStrategy defaultStrategy = ModulationStrategy::EXPONENTIAL ;
};

template <> struct ParameterTypeTraits<ParameterType::AMPLITUDE>{
    using ValueType = double;
    static constexpr ModulationStrategy defaultStrategy = ModulationStrategy::LOGARITHMIC ;
};

template <> struct ParameterTypeTraits<ParameterType::GAIN>{
    using ValueType = double;
    static constexpr ModulationStrategy defaultStrategy = ModulationStrategy::LOGARITHMIC;
};

template <> struct ParameterTypeTraits<ParameterType::PHASE>{
    using ValueType = double;
    static constexpr ModulationStrategy defaultStrategy = ModulationStrategy::ADDITIVE ;
};

template <> struct ParameterTypeTraits<ParameterType::PAN>{
    using ValueType = float;
    static constexpr ModulationStrategy defaultStrategy = ModulationStrategy::ADDITIVE ;
};

template <> struct ParameterTypeTraits<ParameterType::DETUNE>{
    using ValueType = float;
    static constexpr ModulationStrategy defaultStrategy = ModulationStrategy::EXPONENTIAL ;
};

template <> struct ParameterTypeTraits<ParameterType::ATTACK>{
    using ValueType = float;
    static constexpr ModulationStrategy defaultStrategy = ModulationStrategy::EXPONENTIAL ;
};

template <> struct ParameterTypeTraits<ParameterType::DECAY>{
    using ValueType = float;
    static constexpr ModulationStrategy defaultStrategy = ModulationStrategy::EXPONENTIAL ;
};

template <> struct ParameterTypeTraits<ParameterType::SUSTAIN>{
    using ValueType = float;
    static constexpr ModulationStrategy defaultStrategy = ModulationStrategy::ADDITIVE ;
};

template <> struct ParameterTypeTraits<ParameterType::RELEASE>{
    using ValueType = float;
    static constexpr ModulationStrategy defaultStrategy = ModulationStrategy::EXPONENTIAL ;
};

template <> struct ParameterTypeTraits<ParameterType::FILTER_TYPE>{
    using ValueType = int;
    static constexpr ModulationStrategy defaultStrategy = ModulationStrategy::NONE ;
};

template <> struct ParameterTypeTraits<ParameterType::CUTOFF>{
    using ValueType = float;
    static constexpr ModulationStrategy defaultStrategy = ModulationStrategy::EXPONENTIAL ;
};

template <> struct ParameterTypeTraits<ParameterType::Q_FACTOR>{
    using ValueType = float;
    static constexpr ModulationStrategy defaultStrategy = ModulationStrategy::EXPONENTIAL ;
};


#endif // __PARAMETER_TYPE_HPP_