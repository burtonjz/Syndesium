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

    float read(float delay) const {
        // cubic interpolation
        size_t delayInt = static_cast<size_t>(delay);
        float frac = delay - delayInt;
        
        size_t pos0 = (writePos_ + capacity_ - delayInt + 1) % capacity_ ;  
        size_t pos1 = (writePos_ + capacity_ - delayInt) % capacity_ ;      
        size_t pos2 = (writePos_ + capacity_ - delayInt - 1) % capacity_ ;  
        size_t pos3 = (writePos_ + capacity_ - delayInt - 2) % capacity_ ;  
        
        float x0 = buffer_[pos0];
        float x1 = buffer_[pos1];
        float x2 = buffer_[pos2];
        float x3 = buffer_[pos3];
        
        // Hermite interpolation
        float c0 = x1 ;
        float c1 = 0.5f * (x2 - x0);
        float c2 = x0 - 2.5f * x1 + 2.0f * x2 - 0.5f * x3 ;
        float c3 = 0.5f * (x3 - x0) + 1.5f * (x1 - x2);
        
        return ((c3 * frac + c2) * frac + c1) * frac + c0 ;
    }

    void setCapacity(size_t cap){
        buffer_.resize(cap, 0.0f);
        capacity_ = cap ;
    }
};


#endif // DELAY_BUFFER_HPP_