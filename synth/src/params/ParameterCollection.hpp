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

#ifndef PARAMETER_COLLECTION_HPP_
#define PARAMETER_COLLECTION_HPP_

#include "types/ParameterType.hpp"

#include <spdlog/spdlog.h>
#include <vector>

class ParameterCollectionBase {
protected:
    ParameterType type_ ;

public:
    ParameterCollectionBase(
        ParameterType typ
    ):
        type_(typ)
    {}

    ParameterCollectionBase(const ParameterCollectionBase&) = delete;
    ParameterCollectionBase& operator=(const ParameterCollectionBase&) = delete;
};

template<ParameterType typ>
class ParameterCollection : public ParameterCollectionBase {
public:
    using ValueType = GET_PARAMETER_VALUE_TYPE(typ);

private:
    std::vector<ValueType> values_ ;
    ValueType minValue_ ;
    ValueType maxValue_ ;
    std::vector<ValueType> defaultValues_ ;

public:
    ParameterCollection(
        std::vector<ValueType> defaultValues,
        ValueType minValue = ParameterTraits<typ>::minimum,
        ValueType maxValue = ParameterTraits<typ>::maximum
    ):
        ParameterCollectionBase(typ),
        minValue_(minValue),
        maxValue_(maxValue)
    {
        for ( auto v : defaultValues ){
            addValue(v);
        }   
    }

    ParameterCollection():
        ParameterCollection({}){}

    size_t addValue(ValueType v){
        v = limitToRange(v);
        values_.push_back(v);
        defaultValues_.push_back(v);
        return values_.size() - 1 ;
    }

    int removeValue(size_t idx){
        if ( idx >= values_.size() ){
            std::string msg = fmt::format("Cannot remove value from collection. idx {} is out of range {}", idx, values_.size() );
            SPDLOG_ERROR(msg);
            throw std::runtime_error(msg);
        }
        values_.erase(values_.begin() + idx);
        defaultValues_.erase(defaultValues_.begin() + idx);
        return values_.size();
    }

    size_t size() const {
        return values_.size() ;
    }

    ValueType getValue(size_t idx) const {
        if ( idx >= values_.size() ){
            std::string msg = fmt::format("Cannot get value from collection. idx {} is out of range {}", idx, values_.size() );
            SPDLOG_ERROR(msg);
            throw std::runtime_error(msg);
        }
        return values_[idx] ;
    }

    const std::vector<ValueType>& getValues() const {
        return values_ ;
    }

    bool setValue(size_t idx, ValueType v){
        if ( idx >= values_.size() ){
            SPDLOG_ERROR("Cannot set value in collection. idx {} is out of range {}", idx, values_.size() );
            return false ;
        }
        values_[idx] = limitToRange(v);
        return true ;
    }

    ValueType getDefaultValue(size_t idx) const {
        if ( idx >= defaultValues_.size() ){
            std::string msg = fmt::format("Cannot get default value from collection. idx {} is out of range {}", idx, values_.size() );
            SPDLOG_ERROR(msg);
            throw std::runtime_error(msg);
        }
        return defaultValues_[idx] ;
    }

    const std::vector<ValueType>& getDefaultValues() const {
        return defaultValues_ ;
    }

    void setDefaultValue(size_t idx, ValueType v){
        if ( idx >= defaultValues_.size() ){
            SPDLOG_ERROR("Cannot set default value in collection. idx {} is out of range {}", idx, values_.size() );
            return ;
        }
        defaultValues_[idx] = limitToRange(v);
    }

    void setValues(std::vector<ValueType> values){
        values_.clear();
        defaultValues_.clear();
        for ( auto v : values ){
            addValue(v);
        }
    }

    void resetValue(size_t idx){
        if ( idx >= values_.size() ){
            SPDLOG_ERROR("Cannot set value in collection. idx {} is out of range {}", idx, values_.size() );
            return ;
        }
        values_[idx] = defaultValues_[idx] ;
    }

    ValueType getMinValue() const {
        return minValue_ ;
    }

    bool setMinValue(ValueType v){
        if ( v > maxValue_ ){
            SPDLOG_ERROR("Cannot set minimum value higher than maximum value.");
            return false ;
        }
        minValue_ = v ;
        return true ;
    }

    ValueType getMaxValue() const {
        return maxValue_ ;
    }

    bool setMaxValue(ValueType v){
        if ( v < minValue_ ){
            SPDLOG_ERROR("Cannot set maximum value higher than minimum value");
            return false ;
        }
        maxValue_ = v ;
        return true ;
    }

    void setValueRange(ValueType min, ValueType max){
        if ( min > max ){
            SPDLOG_ERROR("Cannot set maximum value higher than minimum value");
            return ;
        }
        minValue_ = min ;
        maxValue_ = max ;
    }

    void reset(){
        values_ = defaultValues_ ;
    }

    void clear(){
        values_.clear();
        defaultValues_.clear();
    }

    void reserve(size_t capacity){
        values_.reserve(capacity);
        defaultValues_.reserve(capacity);
    }

private:
    ValueType limitToRange(ValueType value) const {
            if ( value < minValue_ ) return minValue_ ;
            if ( value > maxValue_ ) return maxValue_ ;
            return value ;
        }
};

#endif // PARAMETER_COLLECTION_HPP_

