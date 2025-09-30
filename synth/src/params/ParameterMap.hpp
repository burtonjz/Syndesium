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
#include "types/ParameterType.hpp"
#include "params/ModulationParameter.hpp"
#include "containers/RTMap.hpp"
#include "containers/AtomicFloat.hpp"
#include "config/Config.hpp"

#include <nlohmann/json.hpp>
#include <variant>
#include <set>
#include <stdexcept>
#include <sstream>
#include <iostream>

#define TYPE_TRAIT(type) typename ParameterTypeTraits<type>::ValueType

// forward declarations
template<ParameterType typ>
class Parameter ;
class ParameterBase ;
class BaseModulator ;

using  ModulationData = RTMap<ModulationParameter, AtomicFloat, N_MODULATION_PARAMETERS> ;
using params = RTMap<ParameterType, ParameterBase*, N_PARAMETER_TYPES> ;
using json = nlohmann::json ;

class ParameterMap {
    private:
        params parameters_ ;
        std::set<ParameterType> modulatable_ ; 
        std::set<ParameterType> reference_ ; // parameters that are reference only to a parent parameter map (like a polyOscillator managing child oscillators)

    public:
        ParameterMap():
            parameters_(),
            modulatable_(),
            reference_()
        {}

        template <ParameterType typ>
        void add(
            TYPE_TRAIT(typ) defaultValue,
            bool modulatable,
            TYPE_TRAIT(typ) minValue = parameterLimits[static_cast<int>(typ)].first, 
            TYPE_TRAIT(typ) maxValue = parameterLimits[static_cast<int>(typ)].second, 
            BaseModulator* modulator = nullptr, ModulationData modData = {}
        ){
            auto it = parameters_.find(typ);
            if (it == parameters_.end()){
                Parameter<typ>* p = new Parameter<typ>(defaultValue, modulatable, minValue, maxValue, modulator, modData);
                parameters_[typ] = p ;
                if (modulatable) modulatable_.insert(typ);
            }
        }

        void addReferences(ParameterMap& other){
            const params& otherParams = other.getBaseMap();
            for (const auto& pair : otherParams ){
                parameters_[pair.first] = pair.second ;
                reference_.insert(pair.first) ;

                // we won't actually perform the modulation in the child object, but we still need to track modulatable parameters
                if(pair.second->isModulatable()) modulatable_.insert(pair.first); 
            }
        }

        template <ParameterType typ>
        TYPE_TRAIT(typ) getValue() const {
            auto it = parameters_.find(typ);
            if ( it != parameters_.end() ){
                return dynamic_cast<Parameter<typ>*>(it->second)->getValue();
            }

            std::stringstream err ;
            err << "ParameterMap:  requested parameter " << static_cast<int>(typ)
                << "does not exist in map." ;
            throw std::runtime_error(err.str());
        }

        template <ParameterType typ>
        void setValue(TYPE_TRAIT(typ) v) const {
            auto it = parameters_.find(typ);
            if ( it != parameters_.end() ){
                dynamic_cast<Parameter<typ>*>(it->second)->setValue(v);
                return ;
            }

            std::stringstream err ;
            err << "ParameterMap:  requested parameter " << static_cast<int>(typ)
                << "does not exist in map." ;
            throw std::runtime_error(err.str());
        }

        template <ParameterType typ>
        TYPE_TRAIT(typ) getInstantaneousValue() const {
            auto it = parameters_.find(typ);
            if ( it != parameters_.end() ){
                return dynamic_cast<Parameter<typ>*>(it->second)->getInstantaneousValue();
            }

            std::stringstream err ;
            err << "ParameterMap:  requested parameter " << static_cast<int>(typ)
                << "does not exist in map." ;
            throw std::runtime_error(err.str());
        }

        /**
         * @brief Get second-order+ modulation depth
         * 
         * @param typ The parameter to get depth for
         * @param depthLevel how deep to delve (0=second-order )
         * @return Parameter<ParameterType::DEPTH>* 
         */
        Parameter<ParameterType::DEPTH>* getParameterDepth(ParameterType typ, int depthLevel){
            auto it = parameters_.find(typ);
            if ( it == parameters_.end()) return nullptr ;

            if ( depthLevel > Config::get<int>("modulation.max_depth")) return nullptr ;

            auto current = it->second->getDepth() ;
            if (!current) return nullptr ;

            // navigate down depth chain
            for ( int i = 1; i < depthLevel && current; ++i){
                current = current->getDepth();
                if (!current) return nullptr ;
            }

            return current ;
        }

        void setModulation(ParameterType typ, BaseModulator* modulator, ModulationData modData){
            auto it = parameters_.find(typ);
            if (it == parameters_.end() ){
                std::cout << "ParameterMap: WARN failed to set modulation for parameter " << static_cast<int>(typ)
                          << " as parameter does not exist in map." << std::endl ;    
                return ;
            }
            it->second->setModulation(modulator, modData);
            it->second->modulate(); // prime modulator
        }

        ModulationData* getModulationData(ParameterType typ){
            auto it = parameters_.find(typ);
            if ( it != parameters_.end() ){
                return it->second->getModulationData() ;
            } else {
                throw std::runtime_error("ParameterMap: WARN modulation data does not exist for specified parameter");
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
        using ParameterValue = std::variant<bool, uint8_t, int, float, double>;
        void setValueDispatch(ParameterType p, const ParameterValue& value){
            switch (p){
                case ParameterType::STATUS: 
                    setValue<ParameterType::STATUS>(std::get<TYPE_TRAIT(ParameterType::STATUS)>(value)); 
                    break ;
                case ParameterType::WAVEFORM: 
                    setValue<ParameterType::WAVEFORM>(std::get<TYPE_TRAIT(ParameterType::WAVEFORM)>(value)); 
                    break ;
                case ParameterType::FREQUENCY: 
                    setValue<ParameterType::FREQUENCY>(std::get<TYPE_TRAIT(ParameterType::FREQUENCY)>(value)); 
                    break ;
                case ParameterType::AMPLITUDE: 
                    setValue<ParameterType::AMPLITUDE>(std::get<TYPE_TRAIT(ParameterType::AMPLITUDE)>(value)); 
                    break ;
                case ParameterType::GAIN: 
                    setValue<ParameterType::GAIN>(std::get<TYPE_TRAIT(ParameterType::GAIN)>(value)); 
                    break ;
                case ParameterType::PHASE: 
                    setValue<ParameterType::PHASE>(std::get<TYPE_TRAIT(ParameterType::PHASE)>(value)); 
                    break ;
                case ParameterType::PAN: 
                    setValue<ParameterType::PAN>(std::get<TYPE_TRAIT(ParameterType::PAN)>(value)); 
                    break ;
                case ParameterType::DETUNE: 
                    setValue<ParameterType::DETUNE>(std::get<TYPE_TRAIT(ParameterType::DETUNE)>(value)); 
                    break ;
                case ParameterType::ATTACK: 
                    setValue<ParameterType::ATTACK>(std::get<TYPE_TRAIT(ParameterType::ATTACK)>(value)); 
                    break ;
                case ParameterType::DECAY: 
                    setValue<ParameterType::DECAY>(std::get<TYPE_TRAIT(ParameterType::DECAY)>(value)); 
                    break ;
                case ParameterType::SUSTAIN: 
                    setValue<ParameterType::SUSTAIN>(std::get<TYPE_TRAIT(ParameterType::SUSTAIN)>(value)); 
                    break ;
                case ParameterType::RELEASE:  
                    setValue<ParameterType::RELEASE>(std::get<TYPE_TRAIT(ParameterType::RELEASE)>(value)); 
                    break ;
                case ParameterType::FILTER_TYPE: 
                    setValue<ParameterType::FILTER_TYPE>(std::get<TYPE_TRAIT(ParameterType::FILTER_TYPE)>(value)); 
                    break ;
                case ParameterType::CUTOFF: 
                    setValue<ParameterType::CUTOFF>(std::get<TYPE_TRAIT(ParameterType::CUTOFF)>(value)); 
                    break ;
                case ParameterType::Q_FACTOR: 
                    setValue<ParameterType::Q_FACTOR>(std::get<TYPE_TRAIT(ParameterType::Q_FACTOR)>(value)); 
                    break ;
                default:
                    throw std::runtime_error("Invalid Parameter dispatch");
            }
        }

        ParameterValue getValueDispatch(ParameterType p) const {
            switch (p){
                case ParameterType::STATUS: 
                    return getValue<ParameterType::STATUS>();
                case ParameterType::WAVEFORM: 
                    return getValue<ParameterType::WAVEFORM>();
                case ParameterType::FREQUENCY: 
                    return getValue<ParameterType::FREQUENCY>();
                case ParameterType::AMPLITUDE: 
                    return getValue<ParameterType::AMPLITUDE>();
                case ParameterType::GAIN: 
                    return getValue<ParameterType::GAIN>();
                case ParameterType::PHASE: 
                    return getValue<ParameterType::PHASE>();
                case ParameterType::PAN: 
                    return getValue<ParameterType::PAN>();
                case ParameterType::DETUNE: 
                    return getValue<ParameterType::DETUNE>();
                case ParameterType::ATTACK: 
                    return getValue<ParameterType::ATTACK>();
                case ParameterType::DECAY: 
                    return getValue<ParameterType::DECAY>();
                case ParameterType::SUSTAIN: 
                    return getValue<ParameterType::SUSTAIN>();
                case ParameterType::RELEASE:  
                    return getValue<ParameterType::RELEASE>();
                case ParameterType::FILTER_TYPE: 
                    return getValue<ParameterType::FILTER_TYPE>();
                case ParameterType::CUTOFF: 
                    return getValue<ParameterType::CUTOFF>();
                case ParameterType::Q_FACTOR: 
                    return getValue<ParameterType::Q_FACTOR>();
                default:
                    throw std::runtime_error("Invalid Parameter dispatch");
            }
        }

        static json dispatchToJson(const ParameterValue& v){
            return std::visit([](auto&& v) -> json {
                if constexpr ( std::is_same_v<std::decay_t<decltype(v)>, uint8_t> ){
                    return static_cast<int>(v);
                } else return v ;
            }, v);
        }


    private:
        void modulateParameter(ParameterType typ){
            auto p = parameters_.find(typ);
            auto r = reference_.find(typ);

            if ( p == parameters_.end() || r != reference_.end() ) return ;

            p->second->modulate();
        }

};

#endif // __PARAMETER_MAP_HPP_