#ifndef __PARAMETER_HPP_
#define __PARAMETER_HPP_

#include "types/ParameterType.hpp"
#include "params/ModulationParameter.hpp"
#include "modulation/Modulator.hpp"
#include "containers/AtomicFloat.hpp"
#include "containers/RTMap.hpp"

class ParameterBase {
protected:
    ParameterType type_ ;
    bool modulatable_ ;
    Modulator* modulator_ ;
    ModulationData modData_ ;
    
public:
    ParameterBase(
        ParameterType typ, 
        bool modulatable,
        Modulator* modulator = nullptr,
        ModulationData modData = {}
    ):
        type_(typ),
        modulatable_(modulatable),
        modulator_(modulator),
        modData_(modData)
    {}

    virtual void resetValue() = 0 ;
    virtual void setModulatable(bool modulatable) = 0 ;
    virtual bool isModulatable() const = 0 ;
    virtual void modulate() = 0 ;

    void setModulation(Modulator* modulator, ModulationData modData){
        modData_ = modData ;
        modulator_ = modulator ;
    }

    ModulationData* getModulationData(){
        return &modData_ ;
    }
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

    public:
        Parameter(
            ValueType defaultValue, bool modulatable, 
            ValueType minValue = parameterLimits[static_cast<int>(typ)].first, 
            ValueType maxValue = parameterLimits[static_cast<int>(typ)].second, 
            Modulator* modulator = nullptr, ModulationData modData = {}
        ):
            ParameterBase(typ,modulatable,modulator,modData),
            minValue_(minValue),
            maxValue_(maxValue),
            value_(limitToRange(defaultValue)),
            instantaneousValue_(value_),
            defaultValue_(value_)
        {}

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

        void setModulatable(bool modulatable){
            modulatable_ = modulatable ;
        }

        bool isModulatable() const {
            return modulatable_ ;
        }

        void modulate(){
            if(!modulatable_ || !modulator_ ) return ;
            setInstantaneousValue(modulator_->modulate(value_, &modData_));
        }
    private:
        void setInstantaneousValue(ValueType v){
            instantaneousValue_ = v ;
        }
};

#endif // __PARAMETER_HPP_