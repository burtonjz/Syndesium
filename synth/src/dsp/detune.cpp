

#include "dsp/detune.hpp"
#include "config/Config.hpp" // or wherever Config is defined
#include <vector>
#include <cmath>

namespace dsp {
    namespace {
        std::vector<double> detuneLUT;
        bool initialized = false;
        size_t maxDetune;
    }
    
    void initializeDetuneLUT() {
        if (initialized) return;
        
        Config::load();
        maxDetune = Config::get<double>("audio.max_detune_cents").value_or(1250);
        
        size_t lutSize = maxDetune * 2 + 1;
        detuneLUT.resize(lutSize);
        
        for (size_t i = 0; i < lutSize; ++i) {
            int c = static_cast<int>(i) - static_cast<int>(maxDetune);
            detuneLUT[i] = std::pow(2.0, c / 1200.0);
        }
        
        initialized = true;
    }
    
    double getDetuneScale(int cents) {
        int index = cents + static_cast<int>(maxDetune);
        if (index < 0 || index >= static_cast<int>(detuneLUT.size())) {
            return 1.0; // No detune if out of range
        }
        return detuneLUT[index];
    }
}