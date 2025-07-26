#ifndef __MODULE_POLYOSCILLATOR_HPP_
#define __MODULE_POLYOSCILLATOR_HPP_

#include "midi/MidiNote.hpp"
#include "modules/BaseModule.hpp"
#include "midi/MidiEventListener.hpp"
#include "containers/RTMap.hpp"
#include "containers/FixedPool.hpp"
#include "modules/Oscillator.hpp"
#include "types/ParameterType.hpp"

#include <cstdint>
#include <vector>

namespace Module {

    class PolyOscillator : public Module::BaseModule, public MidiEventListener {
    private:
        double sampleRate_ ;

        RTMap<uint8_t, Oscillator*, 128> children_ ;
        FixedPool<Oscillator, 128> childPool_ ;

        // modulation
        RTMap<ParameterType, Modulator*, N_PARAMETER_TYPES> modulators_ ;
        RTMap<ParameterType, ModulationData, N_PARAMETER_TYPES> modulationData_ ;
        
    public:
        // Constructors
        PolyOscillator(double sample_rate, std::size_t buf_size, Waveform waveform);
        PolyOscillator(double sample_rate, std::size_t buf_size);

        // getters/setters
        ParameterMap* getParameters();
        
        // Module Overrides
        bool isGenerative() const override ;
        void calculateSample() override ;
        void clearBuffer() override ;
        void tick() override ;

        // MidiEventListener Overrides
        void onKeyPressed(const ActiveNote* note, bool rePress = false) override ;
        void onKeyReleased(ActiveNote anote) override ;
        void onKeyOff(ActiveNote anote) override ;

        void setModulation(ParameterType p, Modulator* m, ModulationData d = {} );

        void updateGain();
    };  
    
}

template <> struct ModuleTypeTraits<ModuleType::PolyOscillator>{using ModType = Module::PolyOscillator;};

#endif // __MODULE_POLYOSCILLATOR_HPP_