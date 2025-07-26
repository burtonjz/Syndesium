#ifndef __MODULE_HPP_
#define __MODULE_HPP_

#include <cmath>
#include <cstddef>
#include <cstring>
#include <memory>
#include <vector>
#include <algorithm>

#include "types/ModuleType.hpp"
#include "params/ParameterMap.hpp"

namespace Module {

class BaseModule {
protected:
    ModuleType  type_ ;
    std::size_t size_ ;
    std::vector<BaseModule*> inputs_ ;
    std::unique_ptr<double[]> buffer_ ;
    size_t bufferIndex_ ;
    ParameterMap parameters_ ;
    

public:
    BaseModule(ModuleType typ, std::size_t size):
        type_(typ),
        size_(size), 
        buffer_(new double[size] ()),
        bufferIndex_(0),
        parameters_()
    {}

    virtual ModuleType getType() const {
        return type_ ;
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

    ParameterMap* getParameters(){
        return &parameters_ ;
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

}

#endif // __MODULE_HPP_