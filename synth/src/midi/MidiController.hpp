#ifndef __MIDI_CONTROLLER_HPP_
#define __MIDI_CONTROLLER_HPP_

#include <set>
#include <array>

#include "midi/MidiEventListener.hpp"
#include "midi/MidiState.hpp"
#include "midi/MidiEventHandler.hpp"

constexpr float CONFIG_PITCHBEND_MAX_SHIFT = 2 ;



class MidiController {
private:
    MidiState* state_ ;
    std::set<MidiEventHandler*> handlers_ ;
       

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

    void addHandler(MidiEventHandler* handler);
    void removeHandler(MidiEventHandler* handler);

    /**
     * @brief tick all midi handlers ( to resolve the event queue)
     * 
     * @param dt delta time
     */
    void tick(float dt);

private:
    void processMessage(double deltaTime, std::vector<unsigned char> *message);

};

#endif // __MIDI_CONTROLLER_HPP_