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

#ifndef __SCALE_NOTE_HPP_
#define __SCALE_NOTE_HPP_

#include <nlohmann/json.hpp>
#include <string>
#include <string_view>
#include <cstdint> 
#include <type_traits>

using json = nlohmann::json ;

class ScaleNote {
public:
    enum Note : uint8_t {
        C = 0, 
        CSHARP_DFLAT = 1, 
        D = 2, 
        DSHARP_EFLAT = 3,
        E = 4, 
        F = 5, 
        FSHARP_GFLAT = 6, 
        G = 7,
        GSHARP_AFLAT = 8, 
        A = 9, 
        ASHARP_BFLAT = 10, 
        B = 11,
        N
    };

    ScaleNote() = default ;

    // construct from enum
    constexpr ScaleNote(Note n) : note_(n){} 

    // construct from string
    ScaleNote(std::string_view name){
        if      ( name == "C" )  note_ = C ;
        else if ( name == "C#" ) note_ = CSHARP_DFLAT ;
        else if ( name == "Db" ) note_ = CSHARP_DFLAT ;
        else if ( name == "D" )  note_ = D ;
        else if ( name == "D#" ) note_ = DSHARP_EFLAT ;
        else if ( name == "Eb" ) note_ = DSHARP_EFLAT ;
        else if ( name == "E" )  note_ = E ;
        else if ( name == "F" )  note_ = F ;
        else if ( name == "F#" ) note_ = FSHARP_GFLAT ;
        else if ( name == "Gb" ) note_ = FSHARP_GFLAT ;
        else if ( name == "G" )  note_ = G ;
        else if ( name == "G#" ) note_ = GSHARP_AFLAT ;
        else if ( name == "Ab" ) note_ = GSHARP_AFLAT ;
        else if ( name == "A" )  note_ = A ;
        else if ( name == "A#" ) note_ = ASHARP_BFLAT ;
        else if ( name == "Bb" ) note_ = ASHARP_BFLAT ;
        else if ( name == "B" )  note_ = B ;
    }

    // allow switch / comparisons
    constexpr operator Note() const { return note_ ; }

    // prevent bool usage e.g., if(Waveform)
    explicit operator bool() const = delete ;

    static std::string toString(Note n, bool preferSharps = true){
        static const std::string sharps[] = 
            {"C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B"};
        static const std::string flats[] = 
            {"C", "Db", "D", "Eb", "E", "F", "Gb", "G", "Ab", "A", "Bb", "B"};

        return preferSharps ? sharps[n] : flats[n] ;
    }

    std::string toString() const {
        return ScaleNote::toString(note_);
    }

    uint8_t getMidiValue(Note n, uint8_t octave, uint8_t interval = 0 ){
        return n + 12 * (octave + 1) + interval ;
    }

    static const std::pair<Note, uint8_t> fromMidiValue(uint8_t midi ){
        uint8_t octave = ( midi / 12 ) - 1 ;
        Note note = static_cast<Note>(modulo(midi, 12));
        return {note, octave};
    }

    static Note from_uint8(uint8_t val){
        return static_cast<Note>(static_cast<std::underlying_type_t<Note>>(val));
    }

    uint8_t to_uint8(){
        return static_cast<uint8_t>(note_) ;
    }

private:
    Note note_ ;

    static int modulo(int a, int b){
        return (a % b + b) % b ;
    }
};

inline void to_json(json& j, const ScaleNote& n){
        j = n.toString();
    }

inline void from_json(const json& j, ScaleNote& n){
    n = ScaleNote(j.get<std::string>());
}

#endif // __SCALE_TYPE_HPP_