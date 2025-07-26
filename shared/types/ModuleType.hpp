#ifndef __MODULE_TYPE_HPP_
#define __MODULE_TYPE_HPP_

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

template <ModuleType Type> struct ModuleTypeTraits ;

#endif // __MODULE_TYPE_HPP_