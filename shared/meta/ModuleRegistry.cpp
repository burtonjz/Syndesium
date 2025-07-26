#include "meta/ModuleRegistry.hpp"
#include "meta/ModuleDescriptor.hpp"
#include "types/ParameterType.hpp"
#include <stdexcept>

const std::unordered_map<ModuleType, ModuleDescriptor>& ModuleRegistry::getAllModuleDescriptors(){
    static const std::unordered_map<ModuleType, ModuleDescriptor> registry = {
        {
            ModuleType::Oscillator, 
            {
                "Oscillator",
                ModuleType::Oscillator,
                {ParameterType::AMPLITUDE, ParameterType::FREQUENCY}, // modulatable params
                {ParameterType::WAVEFORM, ParameterType::AMPLITUDE, ParameterType::FREQUENCY}, //control params
                0, // audio inputs
                1, // audio outputs
                0, // midi inputs
                0, // midi outputs
                false  // polyphonic
            }
        },
        {
            ModuleType::PolyOscillator, 
            {
                "Polyphonic Oscillator",
                ModuleType::PolyOscillator,
                {ParameterType::AMPLITUDE, ParameterType::FREQUENCY, ParameterType::GAIN}, // modulatable params
                {ParameterType::WAVEFORM, ParameterType::AMPLITUDE}, //control params
                0, // audio inputs
                1, // audio outputs
                1, // midi inputs
                0, // midi outputs
                true  // polyphonic
            }
        }

    };

    return registry ;
}

const ModuleDescriptor& ModuleRegistry::getModuleDescriptor(ModuleType type){
    const auto& registry = ModuleRegistry::getAllModuleDescriptors();
    auto  it =  registry.find(type);
    if (it != registry.end()){
        return  it->second ;
    }
    throw std::runtime_error("Unknown ModuleType");
}