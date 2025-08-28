#ifndef __MODULATOR_LINEAR_FADER_HPP_
#define __MODULATOR_LINEAR_FADER_HPP_

#include "modulation/BaseModulator.hpp"
#include "midi/MidiEventHandler.hpp"
#include "types/ModulatorType.hpp"
#include "configs/LinearFaderConfig.hpp"

class LinearFader : public BaseModulator, public MidiEventHandler { 
private:
    KeyMap pending_ ; // MidiModulator holds on to release notes until their release is complete, or the note is pressed again
     
public:
    const static ModulatorType staticType ;

    LinearFader(LinearFaderConfig cfg);
    
    // MODULATOR OVERRIDES
    double modulate(double value, ModulationData* mData) const override;
    
    // MIDI EVENT HANDLER OVERRIDES
    virtual bool shouldKillNote(const ActiveNote& note) const override ;

};

#endif // __MODULATOR_LINEAR_FADER_HPP_