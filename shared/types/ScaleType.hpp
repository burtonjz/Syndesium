/*
 * Copyright (C) 2026 Jared Burton
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

#ifndef __SCALE_TYPE_HPP_
#define __SCALE_TYPE_HPP_

#include <nlohmann/json.hpp>
#include <stdexcept>
#include <string>
#include <string_view>
#include <array>
#include <cstdint> 
#include <type_traits>

using json = nlohmann::json ;

class ScaleType {
public:
    enum Scale : uint8_t {
        MAJOR,
        NATURAL_MINOR,
        HARMONIC_MINOR,
        MELODIC_MINOR,
        PENTATONIC,
        PENTATONIC_MINOR,
        BLUES,
        DORIAN,
        PHRYGIAN,
        LYDIAN,
        MIXOLYDIAN,
        LOCRIAN,
        WHOLE_TONE,
        CHROMATIC,
        DIMINISHED,
        AUGMENTED,
        N
    };

    ScaleType() = default ;

    // construct from enum
    constexpr ScaleType(Scale s) : scale_(s){} 

    // construct from string
    ScaleType(std::string_view name){
        if      ( name == "MAJOR" )            scale_ = MAJOR ;
        else if ( name == "NATURAL_MINOR" )    scale_ = NATURAL_MINOR ;
        else if ( name == "HARMONIC_MINOR" )   scale_ = HARMONIC_MINOR ;
        else if ( name == "MELODIC_MINOR" )    scale_ = MELODIC_MINOR ;
        else if ( name == "PENTATONIC" )       scale_ = PENTATONIC ;
        else if ( name == "PENTATONIC_MINOR" ) scale_ = PENTATONIC_MINOR ;
        else if ( name == "BLUES" )            scale_ = BLUES ;
        else if ( name == "DORIAN" )           scale_ = DORIAN ;
        else if ( name == "PHRYGIAN" )         scale_ = PHRYGIAN ;
        else if ( name == "LYDIAN" )           scale_= LYDIAN ;
        else if ( name == "MIXOLYDIAN" )       scale_ = MIXOLYDIAN ;
        else if ( name == "LOCRIAN" )          scale_ = LOCRIAN ;
        else if ( name == "WHOLE_TONE" )       scale_ = WHOLE_TONE ;
        else if ( name == "CHROMATIC" )        scale_ = CHROMATIC ;
        else if ( name == "DIMINISHED" )       scale_ = DIMINISHED ;
        else if ( name == "AUGMENTED" )        scale_ = AUGMENTED ;
        else throw std::invalid_argument("Unknown scale: " + std::string(name));
    }

    // allow switch / comparisons
    constexpr operator Scale() const { return scale_ ; }

    // prevent bool usage e.g., if(Waveform)
    explicit operator bool() const = delete ;

    static std::string toString(Scale s){
        switch(s){
        case MAJOR:            return "MAJOR" ;
        case NATURAL_MINOR:    return "NATURAL_MINOR" ;
        case HARMONIC_MINOR:   return "HARMONIC_MINOR" ;
        case MELODIC_MINOR:    return "MELODIC_MINOR" ;
        case PENTATONIC:       return "PENTATONIC" ;
        case PENTATONIC_MINOR: return "PENTATONIC_MINOR" ;
        case BLUES:            return "BLUES" ;
        case DORIAN:           return "DORIAN" ;
        case PHRYGIAN:         return "PHRYGIAN" ;
        case LYDIAN:           return "LYDIAN" ;
        case MIXOLYDIAN:       return "MIXOLYDIAN" ;
        case LOCRIAN:          return "LOCRIAN" ;
        case WHOLE_TONE:       return "WHOLE_TONE";
        case CHROMATIC:        return "CHROMATIC";
        case DIMINISHED:       return "DIMINISHED";
        case AUGMENTED:        return "AUGMENTED";
        default:               return "" ;
        };
    }

    std::string toString() const {
        return ScaleType::toString(scale_);
    }

    static std::array<std::string_view, N> getScaleTypes(){
        return { "MAJOR","NATURAL_MINOR","HARMONIC_MINOR","MELODIC_MINOR","PENTATONIC",
            "PENTATONIC_MINOR","BLUES", "DORIAN", "PHRYGIAN", "LYDIAN", "MIXOLYDIAN", "LOCRIAN",
            "WHOLE_TONE","CHROMATIC","DIMINISHED","AUGMENTED" };
    }

    const std::vector<uint8_t>& getIntervals() const {
        static const std::unordered_map<Scale, std::vector<uint8_t>> m = {
            {MAJOR, {0,2,4,5,7,9,11}},
            {NATURAL_MINOR, {0,2,3,5,7,8,10}},
            {HARMONIC_MINOR, {0,2,3,5,7,8,11}},
            {MELODIC_MINOR, {0,2,3,5,7,8,9,10,11}},
            {PENTATONIC, {0,2,4,7,9}},
            {PENTATONIC_MINOR, {0,3,5,7,10}},
            {BLUES, {0,3,5,6,7,10}},
            {DORIAN, {0,2,3,5,7,9,10}},
            {PHRYGIAN, {0,1,3,5,7,8,10}},
            {LYDIAN, {{0,2,4,6,7,9,11}}},
            {MIXOLYDIAN, {0,2,4,5,7,9,10}},
            {LOCRIAN, {0,2,4,5,6,8,10}},
            {WHOLE_TONE, {0,2,4,6,8,10}},
            {CHROMATIC, {0,1,2,3,4,5,6,7,8,9,10,11}},
            {DIMINISHED, {0,2,3,5,6,8,9,11}},
            {AUGMENTED, {0,3,4,7,8,11}}
        };

        return m.at(scale_);
    }

    static Scale from_uint8(uint8_t val){
        return static_cast<Scale>(static_cast<std::underlying_type_t<Scale>>(val));
    }

    uint8_t to_uint8(){
        return static_cast<uint8_t>(scale_) ;
    }

private:
    Scale scale_ ;

};

inline void to_json(json& j, const ScaleType& s){
        j = s.toString();
    }

inline void from_json(const json& j, ScaleType& s){
    s = ScaleType(j.get<std::string>());
}

#endif // __SCALE_TYPE_HPP_