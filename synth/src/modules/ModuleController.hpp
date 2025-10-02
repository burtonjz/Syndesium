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

#ifndef __MODULE_CONTROLLER_HPP_
#define __MODULE_CONTROLLER_HPP_

#include "core/ComponentController.hpp"
#include "modules/SignalChain.hpp"
#include "modules/BaseModule.hpp"
#include "configs/ModuleConfig.hpp"
#include "types/ModuleType.hpp"
#include "types/Waveform.hpp"
#include "config/Config.hpp"

#include "modules/Oscillator.hpp"
#include "modules/PolyOscillator.hpp"

#include <cstddef>
#include <memory>
#include <stdexcept>
#include <utility>
#include <type_traits>
#include <unordered_map>

using ModuleKey = std::pair<ModuleType, int>;

#define HANDLE_CREATE_MODULE(Type) \
    case ModuleType::Type: \
        return create<ModuleType::Type>(name,  j.get<Type##Config>());

struct ModuleKeyHash {
    ComponentId operator()(const ModuleKey& key) const {
        return std::hash<int>()(static_cast<int>(key.first)) ^ (std::hash<int>()(key.second) << 1);
    }
};

class ModuleController: public ComponentController<
    BaseModule,
    ModuleType,
    ModuleKey,
    ModuleKeyHash,
    ModuleTypeTraits
>
{
private:
    SignalChain signalChain_ ;

public:
    ComponentId dispatchFromJson(ModuleType type, const std::string& name, const json& j){
        switch (type){
            HANDLE_CREATE_MODULE(Oscillator)
            HANDLE_CREATE_MODULE(PolyOscillator)
            default:
                throw std::invalid_argument("Unsupported ModuleType");
        }
    }

    // signal chain functions
    void connect(BaseModule* from, BaseModule* to){
        if (!from) return ;
        if (!to) return ;
        to->connectInput(from);
    }

    void registerSink(BaseModule* output){
        signalChain_.addSink(output);
    }

    void unregisterSink(BaseModule* output){
        signalChain_.removeSink(output);
    }

    double processFrame(){
        auto chain = signalChain_.getModuleChain();
        auto sinks = signalChain_.getSinks();

        double output = 0 ; 
        for (BaseModule*  mod : chain){
            mod->tick();
            mod->calculateSample();

            // if it's a sink, add it to the output
            if ( sinks.count(mod) ){
                output += mod->getCurrentSample() ;
            }
        }

        return output ;
    }

    void clearBuffer(){
        for (auto it = components_.begin(); it != components_.end(); ++it){
            if (it->second->isGenerative()){
                it->second->clearBuffer();
            }
        }
    }

    void setup(){
        signalChain_.calculateTopologicalOrder();
    }

    void reset(){
        ComponentController::reset();
        signalChain_.reset() ;
    }
};

#endif // __MODULE_CONTROLLER_HPP_