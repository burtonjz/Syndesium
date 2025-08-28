#ifndef __MODULATOR_ADSR_ENVELOPE_HPP_
#define __MODULATOR_ADSR_ENVELOPE_HPP_

#include "modulation/BaseModulator.hpp"
#include "midi/MidiEventHandler.hpp"
#include "params/ParameterMap.hpp"
#include "configs/ADSREnvelopeConfig.hpp"

class ADSREnvelope : public BaseModulator, public MidiEventHandler { 
private:
    KeyMap pending_ ; // MidiModulator holds on to release notes until their release is complete, or the note is pressed again
    ParameterMap parameters_ ;
     
public:
    const static ModulatorType staticType = ModulatorType::ADSREnvelope ;

    ADSREnvelope(ADSREnvelopeConfig cfg);
    
    // MODULATOR OVERRIDES
    double modulate(double value, ModulationData* mData) const override;
    
    // MIDI EVENT HANDLER OVERRIDES
    virtual bool shouldKillNote(const ActiveNote& note) const override ;

};

#endif // __MODULATOR_ADSR_ENVELOPE_HPP_