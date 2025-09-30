/*
 * Copyright (C) 2025 Jared Burton
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program. If not, see <https://www.gnu.org/licenses/>.
 */

#ifndef __WAVEFORM_HPP_
#define __WAVEFORM_HPP_

#include <nlohmann/json.hpp>
#include <stdexcept>
#include <string>
#include <string_view>
#include <array>
#include <cstdint> 
#include <type_traits>

using json = nlohmann::json ;

class Waveform {
public:
    enum Wave : uint8_t {
        SINE = 0, 
        SQUARE,
        TRIANGLE,
        SAW,
        NOISE,
        N
    };

    Waveform() = default ;
    // construct from enum
    constexpr Waveform(Wave wf) : waveform_(wf){} 

    // construct from string
    Waveform(std::string_view name){
        if      (name == "SINE")     waveform_ = SINE ;
        else if (name == "SQUARE")   waveform_ = SQUARE ;
        else if (name == "TRIANGLE") waveform_ = TRIANGLE ;
        else if (name == "SAW")      waveform_ = SAW ;
        else if (name == "NOISE")    waveform_ = NOISE ;
        else throw std::invalid_argument("Unknown waveform: " + std::string(name));
    }

    // allow switch / comparisons
    constexpr operator Wave() const { return waveform_ ; }

    // prevent bool usage e.g., if(Waveform)
    explicit operator bool() const = delete ;

    static std::string toString(Waveform w){
        switch(w){
        case SINE:     return "SINE" ;
        case SQUARE:   return "SQUARE" ;
        case TRIANGLE: return "TRIANGLE" ;
        case SAW:      return "SAW" ;
        case NOISE:    return "NOISE" ;
        default:       return "" ;
        };
    }

    std::string toString() const {
        return Waveform::toString(waveform_);
    }

    static std::array<std::string_view, N> getWaveforms(){
        return { "SINE", "SQUARE", "TRIANGLE", "SAW", "NOISE"} ;
    }

    static Wave from_uint8(uint8_t val){
        return static_cast<Wave>(static_cast<std::underlying_type_t<Wave>>(val));
    }

    uint8_t to_uint8(){
        return static_cast<uint8_t>(waveform_) ;
    }

private:
    Wave waveform_ ;
};

inline void to_json(json& j, const Waveform& w){
        j = w.toString();
    }

inline void from_json(const json& j, Waveform& w){
    w = Waveform(j.get<std::string>());
}

#endif // __WAVEFORM_HPP_