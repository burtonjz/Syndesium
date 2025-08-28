#ifndef __MODULATOR_TYPE_HPP_
#define __MODULATOR_TYPE_HPP_

/**
 * @brief enumeration of modulation source classes
 * 
*/
enum class ModulatorType {
    LinearFader,
    ADSREnvelope,
    Oscillator,
    MidiControl,
    N_MODULATORS
};

constexpr int N_MODULATOR_TYPES = static_cast<int>(ModulatorType::N_MODULATORS) ;

/* 
every ParameterType will store a default modulation strategy
based on the type of variable it is. This is overridable at the
Parameter level. 
*/ 
enum class ModulationStrategy {
    LINEAR,
    EXPONENTIAL,
    LOGARITHMIC,
    MULTIPLICATIVE,
    NONE
};

// Define a converter from the enum to the actual type. 
// must be defined in every Modulator Header
template <ModulatorType Type> struct ModulatorTypeTraits ;

// define configuration structures for module-specific arguments


#endif // __MODULATOR_TYPE_HPP_