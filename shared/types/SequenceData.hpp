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

#ifndef SEQUENCE_DATA_HPP_
#define SEQUENCE_DATA_HPP_

#include <set>
#include <vector>
#include <cstdint>
#include <algorithm>
#include <iostream>
#include <nlohmann/json.hpp>

struct SequenceNote {
    uint8_t pitch ;
    uint8_t velocity = 100 ;
    float startBeat ; 
    float endBeat ;
    auto operator<=>(const SequenceNote&) const = default ;
};

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(SequenceNote, pitch, velocity, startBeat, endBeat);

class SequenceData {
private:
    std::vector<SequenceNote> notes_ ;

public:
    SequenceData():
        notes_()    
    {}

    void clear(){
        notes_.clear();
    }

    const std::vector<SequenceNote>& get() const {
        return notes_ ;
    }

    void addNote(const SequenceNote& n){
        auto it = std::find(notes_.begin(), notes_.end(), n);
        if ( it != notes_.end() ){
            std::cerr << "Sequence Note was not added to sequence because it is a duplicate." << std::endl ;
            return ;
        }
        notes_.push_back(n);
    }

    void removeNote(const SequenceNote& n){
        auto it = std::find(notes_.begin(), notes_.end(), n);
        if ( it == notes_.end() ){
            std::cerr << "Sequence Note not found in sequence and not removed." << std::endl ;
            return ;
        }
        notes_.erase(it);
    }

    std::set<uint8_t> getActiveNotes(float currentBeat){
        std::set<uint8_t> active ;
        for ( auto& n : notes_ ){
            if ( n.startBeat >= currentBeat && n.endBeat >= currentBeat ){
                active.insert(n.pitch);
            }
        }
        return active ;
    }

};



#endif // SEQUENCE_DATA_HPP_