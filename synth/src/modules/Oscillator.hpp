#ifndef __MODULE_OSCILLATOR_HPP_
#define __MODULE_OSCILLATOR_HPP_

#include "modules/BaseModule.hpp"
#include "types/Waveform.hpp"

namespace Module {

    class Oscillator : public Module::BaseModule {
    private:
        double sampleRate_ ;

        double phase_ ;
        double increment_ ;
        
    public:
        /**
         * @brief Construct a new Oscillator module
         * 
         */
        Oscillator(double sample_rate, Waveform waveform, double frequency, std::size_t buf_size);

        /**
         * @brief Construct a child oscillator module
         * 
        */
        Oscillator(double sample_rate, ParameterMap& map, double frequency, std::size_t buf_size);

        // overrides
        bool isGenerative() const override ;
        void calculateSample() override ; 

        void tick() override ;

        // getters/setters
        void addReferenceParameters(ParameterMap& map);
        void setWaveform(Waveform wave);
        void setFrequency(double freq);
        void setAmplitude(double amp);
     
    };
}

template <> struct ModuleTypeTraits<ModuleType::Oscillator>{using ModType = Module::Oscillator;};

#endif // __MODULE_OSCILLATOR_HPP_