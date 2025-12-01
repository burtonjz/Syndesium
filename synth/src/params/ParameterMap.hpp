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

#ifndef __PARAMETER_MAP_HPP_
#define __PARAMETER_MAP_HPP_

#include "params/Parameter.hpp"
#include "params/ModulationParameter.hpp"
#include "types/ParameterType.hpp"
#include "containers/RTMap.hpp"
#include "containers/AtomicFloat.hpp"
#include "config/Config.hpp"

#include <nlohmann/json.hpp>
#include <variant>
#include <set>
#include <stdexcept>
#include <iostream>


// forward declarations
template<ParameterType typ>
class Parameter ;
class ParameterBase ;
class BaseModulator ;

using  ModulationData = RTMap<ModulationParameter, AtomicFloat, N_MODULATION_PARAMETERS> ;
using params = std::array<ParameterBase*, N_PARAMETER_TYPES> ;
using json = nlohmann::json ;

class ParameterMap {
    private:
        params parameters_{} ;
        std::set<ParameterType> modulatable_ ; 
        std::set<ParameterType> reference_ ; // parameters that are reference only to a parent parameter map (like a polyOscillator managing child oscillators)

    public:
        ParameterMap():
            parameters_(),
            modulatable_(),
            reference_()
        {}

        ParameterBase* getParameter(ParameterType p) const {
            return parameters_[static_cast<size_t>(p)];
        }

        template <ParameterType typ>
        Parameter<typ>* getParameter() const {
            return dynamic_cast<Parameter<typ>*>(getParameter(typ)) ;
        }

        template <ParameterType typ>
        void add(
            GET_PARAMETER_VALUE_TYPE(typ) defaultValue,
            bool modulatable,
            GET_PARAMETER_VALUE_TYPE(typ) minValue = GET_PARAMETER_TRAIT_MEMBER(typ, minimum), 
            GET_PARAMETER_VALUE_TYPE(typ) maxValue = GET_PARAMETER_TRAIT_MEMBER(typ, maximum),
            BaseModulator* modulator = nullptr, ModulationData modData = {}
        ){
            if ( getParameter(typ) ){
                std::cerr << "ERROR: parameter already in map. Exiting..." << std::endl ;
                return ;
            }
                
            Parameter<typ>* p = new Parameter<typ>(defaultValue, modulatable, minValue, maxValue, modulator, modData);
            parameters_[static_cast<size_t>(typ)] = p ;
            if (modulatable) modulatable_.insert(typ);
        }

        void addReferences(ParameterMap& other){
            const params& otherParams = other.getBaseMap();
            for ( size_t i = 0 ; i < otherParams.size() ; ++i ){
                parameters_[i] = otherParams[i] ;
                reference_.insert(ParameterType(i)) ;

                // we won't actually perform the modulation in the child object,
                // but we still need to track modulatable parameters
                if(otherParams[i]->isModulatable()) modulatable_.insert(ParameterType(i)); 
            }
        }

        std::set<ParameterType> getModulatableParameters() const {
            return modulatable_ ;
        }

        const params& getBaseMap() const {
            return parameters_ ;
        }

        void modulate(){
            for (auto it = modulatable_.begin(); it != modulatable_.end(); ++it ){
                modulateParameter(*it);
            }
        }

        // Dispatchers (for API getters/setters)
        ParameterValue getValueDispatch(ParameterType p) const {
            switch (p) {
                #define X(NAME) case ParameterType::NAME: return getParameter<ParameterType::NAME>()->getValue();
                PARAMETER_TYPE_LIST
                #undef X
                default: throw std::runtime_error("Invalid Parameter dispatch");
            }
        }

        ParameterValue getDefaultDispatch(ParameterType p) const {
            switch (p) {
                #define X(NAME) case ParameterType::NAME: return getParameter<ParameterType::NAME>()->getDefaultValue();
                PARAMETER_TYPE_LIST
                #undef X
                default: throw std::runtime_error("Invalid Parameter dispatch");
            }
        }

        ParameterValue getMinDispatch(ParameterType p) const {
            switch (p) {
                #define X(NAME) case ParameterType::NAME: return getParameter<ParameterType::NAME>()->getMinimum();
                PARAMETER_TYPE_LIST
                #undef X
                default: throw std::runtime_error("Invalid Parameter dispatch");
            }
        }

        ParameterValue getMaxDispatch(ParameterType p) const {
            switch (p) {
                #define X(NAME) case ParameterType::NAME: return getParameter<ParameterType::NAME>()->getMaximum();
                PARAMETER_TYPE_LIST
                #undef X
                default: throw std::runtime_error("Invalid Parameter dispatch");
            }
        }

        bool setValueDispatch(ParameterType p, const json& value){
            switch (p){
                #define X(NAME) case ParameterType::NAME: return getParameter<ParameterType::NAME>()->setValue(value);
                PARAMETER_TYPE_LIST
                #undef X
            default:
                return false ;
            }
        }

        bool setDefaultValueDispatch(ParameterType p, const json& value){
            switch (p){
                #define X(NAME) case ParameterType::NAME: return getParameter<ParameterType::NAME>()->setDefaultValue(value);
                PARAMETER_TYPE_LIST
                #undef X
            default:
                return false ;
            }
        }

        bool setMaximumDispatch(ParameterType p, const json& value){
            switch (p){
                #define X(NAME) case ParameterType::NAME: return getParameter<ParameterType::NAME>()->setMaximum(value);
                PARAMETER_TYPE_LIST
                #undef X
            default:
                return false ;
            }
        }

        bool setMinimumDispatch(ParameterType p, const json& value){
            switch (p){
                #define X(NAME) case ParameterType::NAME: return getParameter<ParameterType::NAME>()->setMinimum(value);
                PARAMETER_TYPE_LIST
                #undef X
            default:
                return false ;
            }
        }

        void addParameterDispatch(
            ParameterType p,
            const json& cfg
        ){
            switch(p){
                #define X(NAME) case ParameterType::NAME: \
                    add<ParameterType::NAME>( \
                        cfg["defaultValue"].get<GET_PARAMETER_VALUE_TYPE(ParameterType::NAME)>(), \
                        cfg["modulatable"].get<bool>()                                            \
                    );                                                                            \
                    break ;
                PARAMETER_TYPE_LIST
                #undef X
            default:
                throw std::runtime_error("Invalid Parameter dispatch");
            };
        
        }

        static json ParameterValueToJson(const ParameterValue& v){
            return std::visit([](auto&& v) -> json {
                if constexpr ( std::is_same_v<std::decay_t<decltype(v)>, uint8_t> ){
                    return static_cast<int>(v);
                } else return v ;
            }, v);
        }

        json toJson() const {
            json output ;
            for ( size_t p = 0 ; p < N_PARAMETER_TYPES ; ++p ){
                ParameterType typ = static_cast<ParameterType>(p);
                if (reference_.find(typ) != reference_.end()){
                    continue ; // don't store references
                }
                
                output[GET_PARAMETER_TRAIT_MEMBER(typ, name)] = { 
                    {"currentValue", ParameterValueToJson(getValueDispatch(typ))},
                    {"defaultValue", ParameterValueToJson(getValueDispatch(typ))},
                    {"minimumValue", ParameterValueToJson(getValueDispatch(typ))},
                    {"maximumValue", ParameterValueToJson(getValueDispatch(typ))},
                    {"modulatable", getParameter(typ)->isModulatable()}
                };
            }
            return output ;
        }

        void fromJson(const json& j){
            for (const auto& [name, value] : j.items() ){
                ParameterType p = parameterFromString(name);
                addParameterDispatch(p, value);
            }
        }

    private:
        void modulateParameter(ParameterType typ){
            auto p = getParameter(typ);
            auto r = reference_.find(typ);

            if ( ! p || r != reference_.end() ) return ;

            p->modulate();
        }

};

#endif // __PARAMETER_MAP_HPP_