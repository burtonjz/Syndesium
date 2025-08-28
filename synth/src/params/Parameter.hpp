#ifndef __PARAMETER_HPP_
#define __PARAMETER_HPP_

#include "types/ModulatorType.hpp"
#include "types/ParameterType.hpp"
#include "params/ModulationParameter.hpp"
#include "modulation/BaseModulator.hpp"
#include "containers/AtomicFloat.hpp"
#include "containers/RTMap.hpp"

// forward declaration
template <ParameterType typ> class Parameter ;

class ParameterBase {
protected:
    ParameterType type_ ;
    bool modulatable_ ;
    BaseModulator* modulator_ ;
    std::unique_ptr<Parameter<ParameterType::DEPTH>> modDepth_ ;
    ModulationStrategy modStrategy_ ;
    ModulationData modData_ ;
    
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
        modDepth_(nullptr),
        modStrategy_(ModulationStrategy::NONE),
        modData_(modData)
    {}

    virtual void setModulatable(bool modulatable){
        modulatable_ = modulatable ;
        initializeDepth();
    }

    virtual bool isModulatable(){ 
        return modulatable_ ; 
    }

    void setModulation(BaseModulator* modulator, ModulationData modData){
        modData_ = modData ;
        modulator_ = modulator ;
    }

    // valued functions need to be defined in template class
    virtual void resetValue() = 0 ;
    virtual void modulate() = 0 ;

    ModulationData* getModulationData(){
        return &modData_ ;
    }

    Parameter<ParameterType::DEPTH>* getDepth();

private:
    void initializeDepth();
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
            BaseModulator* modulator = nullptr, ModulationData modData = {}
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

        void modulate(){
            // modulate this parameter's depth, if it exists
            if (modDepth_ && modDepth_->isModulatable()){
                modDepth_->modulate();
            }

            // now, modulate this parameter itself
            if( !modulatable_ || !modulator_ ) return ;
            if( modStrategy_ == ModulationStrategy::NONE ) return ;

            switch(modStrategy_){
            case ModulationStrategy::EXPONENTIAL:
                setInstantaneousValue(value_ * exp2f(modDepth_->getInstantaneousValue() * modulator_->modulate(value_, &modData_)));
                return ;
            case ModulationStrategy::LINEAR:
                setInstantaneousValue(value_ + modDepth_->getInstantaneousValue() * modulator_->modulate(value_, &modData_));
                return ;
            case ModulationStrategy::LOGARITHMIC:
                setInstantaneousValue(value_ * exp2f(modDepth_->getInstantaneousValue() * modulator_->modulate(value_, &modData_)));
                return ;
            case ModulationStrategy::MULTIPLICATIVE:
                setInstantaneousValue(value_ * ( 1 + modDepth_->getInstantaneousValue() * modulator_->modulate(value_, &modData_)));
                return ;
            default:
                return ;

                
            }
        }

    private:
        void setInstantaneousValue(ValueType v){
            instantaneousValue_ = v ;
        }
};

#endif // __PARAMETER_HPP_