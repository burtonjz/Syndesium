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

#define TYPE_TRAIT(type) typename ParameterTraits<type>::ValueType

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
            TYPE_TRAIT(typ) minValue = ParameterTraits<typ>::minimum, 
            TYPE_TRAIT(typ) maxValue = ParameterTraits<typ>::maximum, 
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
        bool setValue(TYPE_TRAIT(typ) v) const {
            auto it = parameters_.find(typ);
            if ( it != parameters_.end() ){
                dynamic_cast<Parameter<typ>*>(it->second)->setValue(v);
                return true ;
            }

            std::cerr << "ParameterMap:  requested parameter " << static_cast<int>(typ)
                      << "does not exist in map." ;
            return false ;
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

        void removeModulation(ParameterType typ){
            auto it = parameters_.find(typ);
            if (it == parameters_.end() ){
                throw std::runtime_error("ParameterMap: ERROR - Parameter does not exist in map.");
            }
            it->second->removeModulation();
        }

        BaseModulator* getModulator(ParameterType typ){
            auto it = parameters_.find(typ);
            if ( it != parameters_.end() ){
                return it->second->getModulator() ;
            } else {
                throw std::runtime_error("ParameterMap: ERROR - Parameter does not exist in map.");
            }
        }

        ModulationData* getModulationData(ParameterType typ){
            auto it = parameters_.find(typ);
            if ( it != parameters_.end() ){
                return it->second->getModulationData() ;
            } else {
                throw std::runtime_error("ParameterMap: ERROR - Parameter does not exist in map.");
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
        bool setValueDispatch(ParameterType p, const json& value){
            switch (p){
                case ParameterType::STATUS: 
                    return setValue<ParameterType::STATUS>(value); 
                case ParameterType::WAVEFORM: 
                    return setValue<ParameterType::WAVEFORM>(value); 
                case ParameterType::FREQUENCY: 
                    return setValue<ParameterType::FREQUENCY>(value); 
                case ParameterType::AMPLITUDE: 
                    return setValue<ParameterType::AMPLITUDE>(value); 
                case ParameterType::GAIN: 
                    return setValue<ParameterType::GAIN>(value); 
                case ParameterType::PHASE: 
                    return setValue<ParameterType::PHASE>(value); 
                case ParameterType::PAN: 
                    return setValue<ParameterType::PAN>(value); 
                case ParameterType::DETUNE: 
                    return setValue<ParameterType::DETUNE>(value); 
                case ParameterType::ATTACK: 
                    return setValue<ParameterType::ATTACK>(value); 
                case ParameterType::DECAY: 
                    return setValue<ParameterType::DECAY>(value); 
                case ParameterType::SUSTAIN: 
                    return setValue<ParameterType::SUSTAIN>(value); 
                case ParameterType::RELEASE:  
                    return setValue<ParameterType::RELEASE>(value); 
                case ParameterType::MIN_VALUE:
                    return setValue<ParameterType::MIN_VALUE>(value); 
                case ParameterType::MAX_VALUE:
                    return setValue<ParameterType::MAX_VALUE>(value); 
                case ParameterType::FILTER_TYPE: 
                    return setValue<ParameterType::FILTER_TYPE>(value); 
                case ParameterType::CUTOFF: 
                    return setValue<ParameterType::CUTOFF>(value); 
                case ParameterType::Q_FACTOR: 
                    return setValue<ParameterType::Q_FACTOR>(value); 
                default:
                    return false ;
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
                case ParameterType::MIN_VALUE:
                    return getValue<ParameterType::MIN_VALUE>(); 
                case ParameterType::MAX_VALUE:
                    return getValue<ParameterType::MAX_VALUE>(); 
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