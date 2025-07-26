#ifndef __MODULE_CONTROLLER_HPP_
#define __MODULE_CONTROLLER_HPP_

#include "modules/SignalChain.hpp"
#include "modules/BaseModule.hpp"
#include "params/Parameter.hpp"
#include "types/ModuleType.hpp"

#include <cstddef>
#include <memory>
#include <utility>
#include <type_traits>
#include <unordered_map>

using ModuleID = std::size_t ;
using ModuleKey = std::pair<ModuleType, int>;

struct ModuleKeyHash {
    std::size_t operator()(const ModuleKey& key) const {
        return std::hash<int>()(static_cast<int>(key.first)) ^ (std::hash<int>()(key.second) << 1);
    }
};

class ModuleController{
private:
    ModuleID nextID_ = 0 ;
    std::unordered_map<ModuleID, std::unique_ptr<Module::BaseModule>> Modules_ ;
    std::unordered_map<ModuleKey, ModuleID, ModuleKeyHash> typeLookup_;
    std::unordered_map<std::string, ModuleID> nameLookup_ ;
    std::unordered_map<ModuleType, int> typeInstanceCount_ ;
    SignalChain signalChain_ ;

public:
    template <ModuleType T, typename... Args>
    ModuleID create(std::string name, Args&&... args){
        using ModType = typename ModuleTypeTraits<T>::ModType ;
        static_assert(std::is_base_of<Module::BaseModule, ModType>::value, "Must be derived from Module");

        int idx = ++typeInstanceCount_[T] ;
        ModuleID id = nextID_++ ;
        
        auto m = std::make_unique<ModType>(std::forward<Args>(args)...);
        Modules_[id] = std::move(m);

        typeLookup_[{T,idx}] = id ;
        nameLookup_[name] = id ;
        
        return id ;
    }

    Module::BaseModule* getRaw(ModuleID id){
        auto it = Modules_.find(id);
        if ( it == Modules_.end() ) return nullptr ;
        return it->second.get() ;
    }

    template<ModuleType T>
    ModuleTypeTraits<T>::ModType* get(ModuleID id){
        Module::BaseModule* m = getRaw(id);
        if (!m) return nullptr ;

        if (m->getType() == T ){
            return static_cast<ModuleTypeTraits<T>::ModType*>(m);
        }

        return nullptr ;
    }

    template<typename T>
    T* getByTypeInstance(int idx){
        ModuleKey key = { T::staticType, idx};
        auto it = typeLookup_.find(key);
        if (it == typeLookup_.end()) return nullptr ;
        return get<T>(it->second) ;
    }

    Module::BaseModule* getByName(const std::string& name){
        auto it = nameLookup_.find(name);
        if ( it == nameLookup_.end() ) return nullptr ;
        return Modules_[it->second].get() ;
    }

    void connect(Module::BaseModule* from, Module::BaseModule* to){
        if (!from) return ;
        if (!to) return ;
        to->connectInput(from);
    }

    void registerSink(Module::BaseModule* output){
        signalChain_.addSink(output);
    }

    void unregisterSink(Module::BaseModule* output){
        signalChain_.removeSink(output);
    }

    double processFrame(){
        auto chain = signalChain_.getModuleChain();
        auto sinks = signalChain_.getSinks();

        double output = 0 ; 
        for (Module::BaseModule*  mod : chain){
            mod->calculateSample();
            // if it's a sink, add it to the output
            if ( sinks.count(mod) ){
                output += mod->getCurrentSample() ;
            }
            mod->tick();
        }

        return output ;
    }

    void clearBuffer(){
        for (auto it = Modules_.begin(); it != Modules_.end(); ++it){
            if (it->second->isGenerative()){
                it->second->clearBuffer();
            }
        }
    }

    void setup(){
        signalChain_.calculateTopologicalOrder();
    }

    void reset(){
        nextID_ = 0 ;
        Modules_.clear() ;
        typeLookup_.clear() ;
        nameLookup_.clear() ;
        typeInstanceCount_.clear() ;
        signalChain_.reset() ;
    }


};

#endif // __MODULE_CONTROLLER_HPP_