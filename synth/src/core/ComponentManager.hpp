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
#include <nlohmann/json.hpp>

using json = nlohmann::json ;

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
    ComponentId create([[maybe_unused]] const std::string& name, ComponentConfig_t<T> cfg) {        
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

    BaseComponent* getRaw(ComponentId id) const {
        auto it = components_.find(id);
        if ( it == components_.end() ) return nullptr ;
        return it->second.get() ;
    }

    template<ComponentType T>
    ComponentType_t<T>* get(ComponentId id) const {
        BaseComponent* m = getRaw(id);
        if (!m) return nullptr ;

        if ( m->getType() == T ){
            return static_cast<ComponentType_t<T>*>(m);
        }

        return nullptr ;
    }

    BaseModule* getModule(ComponentId id) const {
        if ( modules_.find(id) == modules_.end() ) return nullptr ;
        return dynamic_cast<BaseModule*>(getRaw(id));
    }

    std::unordered_set<ComponentId>& getModuleIds(){
        return modules_ ;
    }

    BaseModulator* getModulator(ComponentId id) const {
        if ( modulators_.find(id) == modulators_.end() ) return nullptr ;
        return dynamic_cast<BaseModulator*>(getRaw(id));
    }

    const std::unordered_set<ComponentId>& getModulatorIds() const{
        return modulators_ ;
    }

    MidiEventHandler* getMidiHandler(ComponentId id) const {
        if ( midiHandlers_.find(id) == midiHandlers_.end() ) return nullptr ;
        return dynamic_cast<MidiEventHandler*>(getRaw(id));
    }

    const std::unordered_set<ComponentId>& getMidiHandlerIds() const {
        return midiHandlers_ ;
    }
    
    MidiEventListener* getMidiListener(ComponentId id) const {
        if ( midiListeners_.find(id) == midiListeners_.end() ) return nullptr ;
        return dynamic_cast<MidiEventListener*>(getRaw(id));
    }

    const std::unordered_set<ComponentId>& getMidiListenerIds() const {
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

    // saving / loading
    json serializeComponents() const {
        json output ;
        for ( auto it = components_.begin(); it != components_.end(); ++it ){
            json componentConfig ;

            componentConfig["id"] = it->second->getId() ;
            componentConfig["type"] = it->second->getType() ;

            // parameters
            componentConfig["parameters"] = it->second->getParameters()->toJson();

            // get any additional modulations
            auto modulatableParameters = ComponentRegistry::getComponentDescriptor(it->second->getType()).modulatableParameters ;
            for ( auto p : modulatableParameters ){
                auto modulator = it->second->getParameterModulator(p);
                if ( modulator ){
                    componentConfig["parameters"][GET_PARAMETER_TRAIT_MEMBER(p, name)]["modulatorId"] = modulator->getId();
                }
            }

            // get input signal component ids
            BaseModule* module = dynamic_cast<BaseModule*>(it->second.get());
            if ( module ){
                for ( auto c : module->getInputs() ){
                    componentConfig["signalInputs"].push_back(c->getId());
                }
            }

            // get midi handler listeners
            MidiEventHandler* handler = dynamic_cast<MidiEventHandler*>(it->second.get());
            if ( handler ){
                for ( auto listener : handler->getListeners() ){
                    auto listenerModule = dynamic_cast<BaseComponent*>(listener);
                    if ( listenerModule ){
                        componentConfig["midiListeners"].push_back(listenerModule->getId());
                    }
                        
                }
            }
            
            output.push_back(componentConfig);

            componentConfig.clear();
        }
        return output ;
    }
};

#endif // __COMPONENT_MANAGER_HPP_
