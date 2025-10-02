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

#ifndef __COMPONENT_CONTROLLER_HPP_
#define __COMPONENT_CONTROLLER_HPP_

#include "params/ParameterMap.hpp"
#include "types/ParameterType.hpp"
#include <memory>
#include <unordered_map>

using ComponentId = int ;

template <
    typename BaseType, 
    typename TypeEnum, 
    typename KeyType, 
    typename KeyHash,
    template<TypeEnum> class TypeTraits
>
class ComponentController {
protected:
    template<TypeEnum T>
    using ConcreteType_t = typename TypeTraits<T>::type ;
    
    template<TypeEnum T>
    using ConfigType_t = typename TypeTraits<T>::config ;

    int nextID_ = 0 ;
    std::unordered_map<ComponentId, std::unique_ptr<BaseType>> components_ ;
    std::unordered_map<KeyType, ComponentId, KeyHash> typeLookup_;
    std::unordered_map<std::string, ComponentId> nameLookup_ ;
    std::unordered_map<TypeEnum, int> typeInstanceCount_ ;

public:
    // Generic create method
    template<TypeEnum T>
    ComponentId create(const std::string& name, ConfigType_t<T> cfg) {
        static_assert(std::is_base_of<BaseType, ConcreteType_t<T>>::value, 
                      "ConcreteType must derive from BaseType");
        
        int idx = ++typeInstanceCount_[T];
        ComponentId id = nextID_++;
        
        auto component = std::make_unique<ConcreteType_t<T>>(cfg);
        components_[id] = std::move(component);
        typeLookup_[{T, idx}] = id;
        nameLookup_[name] = id;
        
        return id;
    }

    BaseType* getRaw(ComponentId id){
        auto it = components_.find(id);
        if ( it == components_.end() ) return nullptr ;
        return it->second.get() ;
    }

    template<TypeEnum T>
    ConcreteType_t<T>* get(ComponentId id){
        BaseType* m = getRaw(id);
        if (!m) return nullptr ;

        if ( m->getType() == T ){
            return static_cast<ConcreteType_t<T>*>(m);
        }

        return nullptr ;
    }

    template<typename T>
    T* getByTypeInstance(int idx){
        KeyType key = { T::staticType, idx};
        auto it = typeLookup_.find(key);
        if (it == typeLookup_.end()) return nullptr ;
        return get<T>(it->second) ;
    }

    BaseType* getByName(const std::string& name){
        auto it = nameLookup_.find(name);
        if ( it == nameLookup_.end() ) return nullptr ;
        return components_[it->second].get() ;
    }

    void connect(BaseType* from, BaseType* to){
        if (!from) return ;
        if (!to) return ;
        to->connectInput(from);
    }

    void clearBuffer(){
        for (auto it = components_.begin(); it != components_.end(); ++it){
            if (it->second->isGenerative()){
                it->second->clearBuffer();
            }
        }
    }

    void reset(){
        nextID_ = 0 ;
        components_.clear() ;
        typeLookup_.clear() ;
        nameLookup_.clear() ;
        typeInstanceCount_.clear() ;
    }

    bool setComponentParameter(ComponentId id, ParameterType t, const ParameterValue& value){
        auto component = getRaw(id);
        if (!component){
            std::cerr << "WARN: module with id " << id << " not found in controller.\n" ;
            return false ;
        }

        return component->setParameterValue(t, value);
    }
};

#endif // __COMPONENT_CONTROLLER_HPP_
