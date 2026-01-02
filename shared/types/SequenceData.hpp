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

#include <vector>
#include <cstdint>
#include <algorithm>
#include <spdlog/spdlog.h>
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
    float lastQueriedBeat_ ;

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
            SPDLOG_WARN("Sequence Note was not added to sequence because it is a duplicate.");
            return ;
        }
        notes_.push_back(n);
    }

    void removeNote(const SequenceNote& n){
        auto it = std::find(notes_.begin(), notes_.end(), n);
        if ( it == notes_.end() ){
            SPDLOG_WARN("Sequence Note not found in sequence and not removed.");
            return ;
        }
        notes_.erase(it);
    }

    template<typename Func>
    void processEvents(float currentBeat, float loopLength, Func&& callback){
        // handle loop around
        if ( currentBeat < lastQueriedBeat_ ){
            for ( const auto& n : notes_ ){
                // Events from last query to loop end
                if ( n.startBeat > lastQueriedBeat_ && n.startBeat <= loopLength ){
                    callback(true, n);
                }
                if ( n.endBeat > lastQueriedBeat_ && n.endBeat <= loopLength ){
                    callback(false, n);
                }

                // Events from loop start to current beat
                if ( n.startBeat >= 0.0f && n.startBeat <= currentBeat ){
                    callback(true, n);
                }
                if ( n.endBeat >= 0.0f && n.endBeat <= currentBeat ){
                    callback(false, n);
                }
            }
        } else {
            for ( const auto& n : notes_ ){
                if ( n.startBeat > lastQueriedBeat_ && n.startBeat <= currentBeat ){
                    callback(true, n);
                }
                if ( n.endBeat > lastQueriedBeat_ && n.endBeat <= currentBeat ){
                    callback(false, n);
                }
            }
        }
        lastQueriedBeat_ = currentBeat ;
    }

};



#endif // SEQUENCE_DATA_HPP_