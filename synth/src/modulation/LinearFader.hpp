#ifndef __MODULATOR_LINEAR_FADER_HPP_
#define __MODULATOR_LINEAR_FADER_HPP_

#include "modulation/Modulator.hpp"
#include "midi/MidiEventHandler.hpp"
#include "midi/MidiState.hpp"
#include "types/ModulatorType.hpp"


class LinearFader : public Modulator, public MidiEventHandler { 
private:
    const MidiState* state_  ;
    KeyMap pending_ ; // MidiModulator holds on to release notes until their release is complete, or the note is pressed again
     
public:
    const static ModulatorType staticType ;

    LinearFader(MidiState* state);
    
    // MODULATOR OVERRIDES
    double modulate(double value, ModulationData* mData) const override;
    
    // MIDI EVENT HANDLER OVERRIDES
    virtual bool shouldKillNote(const ActiveNote& note) const override ;

};

template <> struct ModulatorTypeTraits<ModulatorType::LinearFader>{using ModType = LinearFader;};

#endif // __MODULATOR_LINEAR_FADER_HPP_