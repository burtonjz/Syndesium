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

#ifndef __PARAMETER_HPP_
#define __PARAMETER_HPP_

#include "types/ParameterType.hpp"
#include "core/BaseModulator.hpp"
#include "containers/RTMap.hpp"
#include <iostream>
#include <variant>

// forward declaration
template <ParameterType typ> class Parameter ;

class ParameterBase {
protected:
    ParameterType type_ ;
    bool modulatable_ ;
    BaseModulator* modulator_ ;
    ModulationStrategy modStrategy_ ;
    ModulationData modData_ ;

    bool hasDepth_ ;
    bool depthInitialized_ ;
    
public:
    ParameterBase(
        ParameterType typ, 
        bool modulatable,
        BaseModulator* modulator = nullptr,
        ModulationData modData = {}
    ):
        type_(typ),
        modulatable_(modulatable),
        modulator_(modulator),
        modStrategy_(ModulationStrategy::NONE),
        modData_(modData)    
    {}

    ParameterBase(const ParameterBase&) = delete;
    ParameterBase& operator=(const ParameterBase&) = delete;

    virtual void setModulatable(bool modulatable){
        modulatable_ = modulatable ;
    }

    virtual bool isModulatable(){ 
        return modulatable_ ; 
    }

    void setModulation(BaseModulator* modulator, ModulationData modData){
        if ( modulator){
            modData_ = modData ;
            modulator_ = modulator ;
        } else {
            std::cerr << "WARN: attempted to set modulator, but the pointer is null." << std::endl;
        }
        
    }

    ModulationData* getModulationData(){
        return &modData_ ;
    }

    virtual void resetValue() = 0 ;
    virtual void modulate() = 0 ;
    virtual Parameter<ParameterType::DEPTH>* getDepth(){ return nullptr ;}

};

template <ParameterType typ>
struct DepthStorage {
    static constexpr size_t DEPTH_SLOT_SIZE = 512 ;
};

template <ParameterType typ>
class Parameter : public ParameterBase {
    public:
        using ValueType = typename ParameterTypeTraits<typ>::ValueType ;

    private:
        ValueType minValue_ ;
        ValueType maxValue_ ;
        ValueType value_ ;
        ValueType instantaneousValue_ ;
        ValueType defaultValue_ ;

        // fixed size container for a Parameter<ParameterType::DEPTH>, only on non-DEPTH parameters
        static constexpr size_t DEPTH_SLOT_SIZE = 512; 

        struct AlignedStorage {
            alignas(std::max_align_t) std::byte buf[DEPTH_SLOT_SIZE];
        };

        using DepthStorage = std::conditional_t<
            typ == ParameterType::DEPTH,
            std::monostate,
            AlignedStorage
        >;

        DepthStorage depthStorage_ ;
        Parameter<ParameterType::DEPTH>* depth_ = nullptr ;
        
    public:
        Parameter(
            ValueType defaultValue, bool modulatable, 
            ValueType minValue = parameterLimits[static_cast<int>(typ)].first, 
            ValueType maxValue = parameterLimits[static_cast<int>(typ)].second, 
            BaseModulator* modulator = nullptr, ModulationData modData = {}
        ):
            ParameterBase(typ,modulatable,modulator,modData),
            minValue_(minValue),
            maxValue_(maxValue),
            value_(limitToRange(defaultValue)),
            instantaneousValue_(value_),
            defaultValue_(value_)
        {
            modStrategy_ = ParameterTypeTraits<typ>::defaultStrategy ;

            if constexpr (typ != ParameterType::DEPTH){
                depth_ = new (depthStorage_.buf) Parameter<ParameterType::DEPTH>(
                    1.0f,
                    true
                );
            }
        }

        ~Parameter(){
            if constexpr (typ != ParameterType::DEPTH){
                if (depth_){
                    depth_->~Parameter();
                    depth_ = nullptr ;
                }
            }
        }
        Parameter(const Parameter&) = delete ;
        Parameter& operator=(const Parameter&) = delete ;
        Parameter(Parameter&&) = delete ;
        Parameter& operator=(Parameter&&) = delete ;

        /**
        * @brief limit value to Parameter's range
        * 
        * @param value value
        */
        ValueType limitToRange(ValueType value) const {
            if ( value < minValue_ ) return minValue_ ;
            if ( value > maxValue_ ) return maxValue_ ;
            return value ;
        }

        void setValue(ValueType value){
            value_ = limitToRange(value);
            setInstantaneousValue(value_);
        }

        void resetValue(){
            setValue(defaultValue_);
        }

        ValueType getValue() const {
            return value_ ;
        }

        ValueType getInstantaneousValue() const {
            return instantaneousValue_ ;
        }

        void modulate(){
            // modulate this parameter's depth, if it exists
            auto* depth = getDepth();
            if ( depth  && depth->isModulatable()){
                getDepth()->modulate();
            }

            // now, modulate this parameter itself
            if( !modulatable_ || !modulator_ ) return ;
            if( modStrategy_ == ModulationStrategy::NONE ) return ;

            switch(modStrategy_){
            case ModulationStrategy::ADDITIVE:
                setInstantaneousValue(value_ + getDepth()->getInstantaneousValue() * modulator_->modulate(value_, &modData_));
                return ;
            case ModulationStrategy::MULTIPLICATIVE:
                setInstantaneousValue( value_ * getDepth()->getInstantaneousValue() * modulator_->modulate(value_, &modData_));
                return ;
            case ModulationStrategy::EXPONENTIAL:
                setInstantaneousValue(value_ * exp2f(getDepth()->getInstantaneousValue() * modulator_->modulate(value_, &modData_)));
                return ;
            case ModulationStrategy::LOGARITHMIC:
                {
                    float mout = modulator_->modulate(value_, &modData_);
                    if ( mout <= 0.0f ){
                        setInstantaneousValue(0.0f);
                    } else {
                        // map 0-1 range to db (-60db to 0db)
                        float db = -60.0f + 60.0f * mout ;
                        setInstantaneousValue(value_ * pow(10.0f, db / 20.0f));
                    }
                }
                return ;
            case ModulationStrategy::REPLACE:
                setInstantaneousValue(getDepth()->getInstantaneousValue() * modulator_->modulate(value_, &modData_));
                return ;
            default:
                return ;
            }
        }

        Parameter<ParameterType::DEPTH>* getDepth(){
            return depth_ ;
        }

    private:
        void setInstantaneousValue(ValueType v){
            instantaneousValue_ = v ;
        }
};


#endif // __PARAMETER_HPP_