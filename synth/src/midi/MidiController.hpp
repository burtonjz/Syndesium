#ifndef __MIDI_CONTROLLER_HPP_
#define __MIDI_CONTROLLER_HPP_

#include <vector>
#include <array>

#include "midi/MidiState.hpp"

constexpr float CONFIG_PITCHBEND_MAX_SHIFT = 2 ;



class MidiController {
private:
    MidiState* state_ ;
       

public:
    MidiController(MidiState* state);

    void initialize() ;

    /**
     * @brief Callback function. 
     * 
     * @param deltaTime time past since last midi event
     * @param message midi message
     * @param userData pointer to this class
     */
    static void onMidiEvent(double deltaTime, std::vector<unsigned char> *message, void *userData);

    static std::array<double,16384> pitchbendScaleFactor_ ;
    static void computePitchbendScaleFactor();

private:
    void processMessage(double deltaTime, std::vector<unsigned char> *message);

};

#endif // __MIDI_CONTROLLER_HPP_