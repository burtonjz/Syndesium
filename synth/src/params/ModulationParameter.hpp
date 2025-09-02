#ifndef __MODULATION_PARAMETER_HPP_
#define __MODULATION_PARAMETER_HPP_

/**
 * @brief valid key values for handling additional modulation variables
 * 
 * The parameter class stores a modulation function in it that can receive a map as a parameter so that additional context can be given to modulators
 * while still allowing the modulation function definition to be standardized. These keys define various modulation function variables.
*/
enum class ModulationParameter {
    MIDI_NOTE,
    INITIAL_VALUE, 
    LAST_VALUE,
    N_PARAMETERS
};

constexpr int N_MODULATION_PARAMETERS = static_cast<int>(ModulationParameter::N_PARAMETERS);

#endif // __MODULATION_PARAMETER_HPP_