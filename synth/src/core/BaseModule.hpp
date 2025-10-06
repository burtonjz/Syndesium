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

#ifndef __MODULE_HPP_
#define __MODULE_HPP_

#include <cmath>
#include <cstddef>
#include <cstring>
#include <memory>
#include <vector>
#include <algorithm>

#include "core/BaseComponent.hpp"
#include "types/ComponentType.hpp"
#include "params/ParameterMap.hpp"

class BaseModule : public virtual BaseComponent {
protected:
    double  sampleRate_ ;
    std::size_t size_ ;
    std::vector<BaseModule*> inputs_ ;
    std::unique_ptr<double[]> buffer_ ;
    size_t bufferIndex_ ;
    

public:
    BaseModule():
        bufferIndex_(0)
    {
        Config::load();
        double sampleRate = Config::get<double>("audio.sample_rate").value();
        size_t bufferSize = Config::get<size_t>("audio.buffer_size").value();

        sampleRate_ = sampleRate ;
        size_ = bufferSize ;
        buffer_ = std::make_unique<double[]>(size_) ;
    }
    
    virtual ~BaseModule() = default ;

    void setBufferIndex(size_t index){
        bufferIndex_ = index ;
    }
    
    virtual void calculateSample(){}

    double getCurrentSample() const {
        return buffer_[bufferIndex_];
    }

    virtual void clearBuffer(){
        std::fill(buffer_.get(), buffer_.get() + size_, 0.0);
    }

    const double* data() const {
        return buffer_.get() ;
    }

    std::size_t size() const {
        return size_ ;
    }

    void connectInput(BaseModule* source){
        if ( std::find(inputs_.begin(), inputs_.end(), source) == inputs_.end() ){
            std::cerr << "WARN: module already in input list. Not adding again." << std::endl ;
            return ;
        }
        inputs_.push_back(source);
    }

    void disconnectInput(BaseModule* source){
        auto it = std::find(inputs_.begin(), inputs_.end(), source);
        if (  it == inputs_.end() ){
            std::cerr << "WARN: module not in input list. Not able to remove." << std::endl ;
            return ;
        }
        inputs_.erase(it);
    }

    const std::vector<BaseModule*>& getInputs() const {
        return inputs_ ;
    }

    virtual void tick(){
        bufferIndex_ = std::fmod(bufferIndex_ + 1, size_);
    }

    virtual bool isGenerative() const { return false; }
    virtual bool isPolyphonic() const { return false; }

protected:
    virtual void processBuffer(double* buf, size_t len){} 
    virtual void processBuffer(){}
    
};

#endif // __MODULE_HPP_