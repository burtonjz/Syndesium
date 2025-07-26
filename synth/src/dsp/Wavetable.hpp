#ifndef __WAVETABLE_HPP_
#define __WAVETABLE_HPP_

#include "types/Waveform.hpp"

#include <mutex>
#include <array>
#include <vector>
#include <utility>

using WaveMap = std::array<std::vector<double>, static_cast<int>(Waveform::N)> ;
using Wave = std::pair<double*, std::size_t> ;

/**
 * class stores all wavetables available to an Oscillator
*/
class Wavetable {
private:
    static std::once_flag initFlag_ ;
    static WaveMap waves_ ;

public:
    /**
     * @brief generates all wavetables
    */
    static void generate();

    /**
     * @brief return a read-only pointer to the waveform's wavetable
    */
    static const Wave getWavetable(Waveform waveform);

private:
    static void generateSineWavetable();
    static void generateSquareWavetable();
    static void generateTriangleWavetable();
    static void generateSawWavetable();
    static void generateNoiseWavetable();

    static double polyBlep(double t, double dt);
};

#endif // __WAVETABLE_HPP_