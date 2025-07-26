#ifndef __MODULATOR_ADSR_ENVELOPE_HPP_
#define __MODULATOR_ADSR_ENVELOPE_HPP_

#include "modulation/Modulator.hpp"
#include "midi/MidiEventHandler.hpp"
#include "midi/MidiState.hpp"
#include "params/ParameterMap.hpp"

class ADSREnvelope : public Modulator, public MidiEventHandler { 
private:
    const MidiState* state_  ;
    KeyMap pending_ ; // MidiModulator holds on to release notes until their release is complete, or the note is pressed again
    ParameterMap parameters_ ;
     
public:
    const static ModulatorType staticType = ModulatorType::ADSREnvelope ;

    ADSREnvelope(MidiState* state);
    
    // MODULATOR OVERRIDES
    double modulate(double value, ModulationData* mData) const override;
    
    // MIDI EVENT HANDLER OVERRIDES
    virtual bool shouldKillNote(const ActiveNote& note) const override ;

};

template <> struct ModulatorTypeTraits<ModulatorType::ADSREnvelope>{using ModType = ADSREnvelope;};

#endif // __MODULATOR_ADSR_ENVELOPE_HPP_