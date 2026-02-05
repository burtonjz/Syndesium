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

#ifndef DELAY_BUFFER_HPP_
#define DELAY_BUFFER_HPP_

#include <vector>

class DelayBuffer {
private:
    std::vector<float> buffer_ ;
    size_t writePos_ = 0 ;
    size_t capacity_ ;

public:
    DelayBuffer(size_t maxSamples):
        buffer_(maxSamples, 0.0f),
        capacity_(maxSamples)
    {}

    void write(float sample){
        buffer_[writePos_] = sample ;
        writePos_ = (writePos_ + 1) % capacity_ ;
    }

    float read(size_t delay) const {
        if ( delay >= capacity_ ){
            delay = capacity_ - 1 ;
        }
        size_t idx = (writePos_ + capacity_ - delay) % capacity_ ;
        return buffer_[idx] ;
    }

    void setCapacity(size_t cap){
        buffer_.resize(cap, 0.0f);
        capacity_ = cap ;
    }
};


#endif // DELAY_BUFFER_HPP_