#ifndef __MODULE_OSCILLATOR_HPP_
#define __MODULE_OSCILLATOR_HPP_

#include "modules/BaseModule.hpp"
#include "configs/OscillatorConfig.hpp"
#include "types/Waveform.hpp"

class Oscillator : public BaseModule {
private:
    double phase_ ;
    double increment_ ;
    
public:
    /**
        * @brief Construct a new Oscillator module
        * 
        */
    Oscillator(double sample_rate, std::size_t buf_size, OscillatorConfig cfg);

    /**
        * @brief Construct a child oscillator module
        * 
    */
    Oscillator(double sample_rate, std::size_t buf_size, ParameterMap& map, double frequency);

    // overrides
    bool isGenerative() const override ;
    void calculateSample() override ; 

    void tick() override ;
    void reset();
    
    // getters/setters
    void addReferenceParameters(ParameterMap& map);
    void setWaveform(Waveform wave);
    void setFrequency(double freq);
    void setAmplitude(double amp);
    
};

#endif // __MODULE_OSCILLATOR_HPP_