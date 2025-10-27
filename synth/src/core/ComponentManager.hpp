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

#ifndef __COMPONENT_MANAGER_HPP_
#define __COMPONENT_MANAGER_HPP_

#include "core/BaseComponent.hpp"
#include "core/BaseModule.hpp"
#include "core/BaseModulator.hpp"
#include "midi/MidiController.hpp"
#include "midi/MidiEventHandler.hpp"

#include "params/ParameterMap.hpp"
#include "types/ParameterType.hpp"
#include "types/ComponentType.hpp"

#include "meta/ComponentRegistry.hpp"

#include <memory>
#include <unordered_map>
#include <unordered_set>

class ComponentManager {
private:
    MidiController* midiController_ ;
    int nextID_ = 0 ;
    std::unordered_map<ComponentId, std::unique_ptr<BaseComponent>> components_ ;
    
    // "views" of different component groups
    std::unordered_set<ComponentId> midiHandlers_ ;
    std::unordered_set<ComponentId> midiListeners_ ;
    std::unordered_set<ComponentId> modulators_ ;
    std::unordered_set<ComponentId> modules_ ;

public:
    ComponentManager(MidiController* midiCtrl):
        midiController_(midiCtrl)
    {}

    template<ComponentType T>
    ComponentId create(const std::string& name, ComponentConfig_t<T> cfg) {        
        ComponentId id = nextID_++;
        
        components_.emplace(id, std::make_unique<ComponentType_t<T>>(id, cfg));

        auto descriptor = ComponentRegistry::getComponentDescriptor(T);
        if ( descriptor.isModule() ) modules_.insert(id);
        if ( descriptor.isModulator() ) modulators_.insert(id);
        if ( descriptor.isMidiListener() ) midiListeners_.insert(id);
        if ( descriptor.isMidiHandler() ){
            midiHandlers_.insert(id);
            midiController_->addHandler(getMidiHandler(id));
        } 

        return id;
    }

    BaseComponent* getRaw(ComponentId id){
        auto it = components_.find(id);
        if ( it == components_.end() ) return nullptr ;
        return it->second.get() ;
    }

    template<ComponentType T>
    ComponentType_t<T>* get(ComponentId id){
        BaseComponent* m = getRaw(id);
        if (!m) return nullptr ;

        if ( m->getType() == T ){
            return static_cast<ComponentType_t<T>*>(m);
        }

        return nullptr ;
    }

    BaseModule* getModule(ComponentId id){
        if ( modules_.find(id) == modules_.end() ) return nullptr ;
        return dynamic_cast<BaseModule*>(getRaw(id));
    }

    std::unordered_set<ComponentId>& getModuleIds(){
        return modules_ ;
    }

    BaseModulator* getModulator(ComponentId id){
        if ( modulators_.find(id) == modulators_.end() ) return nullptr ;
        return dynamic_cast<BaseModulator*>(getRaw(id));
    }

    std::unordered_set<ComponentId>& getModulatorIds(){
        return modulators_ ;
    }

    MidiEventHandler* getMidiHandler(ComponentId id){
        if ( midiHandlers_.find(id) == midiHandlers_.end() ) return nullptr ;
        return dynamic_cast<MidiEventHandler*>(getRaw(id));
    }

    std::unordered_set<ComponentId>& getMidiHandlerIds(){
        return midiHandlers_ ;
    }
    
    MidiEventListener* getMidiListener(ComponentId id){
        if ( midiListeners_.find(id) == midiListeners_.end() ) return nullptr ;
        return dynamic_cast<MidiEventListener*>(getRaw(id));
    }

    std::unordered_set<ComponentId>& getMidiListenerIds(){
        return midiListeners_ ;
    }

    void remove(ComponentId id){
        midiHandlers_.erase(id);
        modules_.erase(id);
        modulators_.erase(id);

        components_.erase(id);
    }

    void reset(){
        nextID_ = 0 ;
        components_.clear();
        midiHandlers_.clear();
        modulators_.clear();
        modules_.clear();
    }

    bool setComponentParameter(ComponentId id, ParameterType t, const json& value){
        auto component = getRaw(id);
        if (!component){
            std::cerr << "WARN: component with id " << id << " not found in component store.\n" ;
            return false ;
        }

        return component->setParameterValue(t, value);
    }

    void runParameterModulation(){
        for (auto it = components_.begin(); it != components_.end(); ++it){
            it->second->updateParameters();
        }
    }
};

#endif // __COMPONENT_MANAGER_HPP_
