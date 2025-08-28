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

// Define a converter from the enum to the actual type. 
// must be defined in synth/configs/
template <ModuleType Type> struct ModuleTypeTraits ;

// helper aliases
template<ModuleType Type>
using ModuleType_t = typename ModuleTypeTraits<Type>::type ;

template<ModuleType Type>
using ModuleConfig_t = typename ModuleTypeTraits<Type>::config ;

#endif // __MODULE_TYPE_HPP_