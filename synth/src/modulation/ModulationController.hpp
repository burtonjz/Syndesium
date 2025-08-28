#ifndef __MODULATION_CONTROLLER_HPP_
#define __MODULATION_CONTROLLER_HPP_

#include "midi/MidiEventHandler.hpp"
#include "modulation/BaseModulator.hpp"
#include "modulation/LinearFader.hpp"
#include "modulation/ADSREnvelope.hpp"
#include "configs/ModulatorConfig.hpp"

#include <cstddef>
#include <memory>
#include <utility>
#include <type_traits>
#include <unordered_map>

using ModulatorID = std::size_t ;
using ModulatorKey = std::pair<ModulatorType, int>;

struct ModulatorKeyHash {
    std::size_t operator()(const ModulatorKey& key) const {
        return std::hash<int>()(static_cast<int>(key.first)) ^ (std::hash<int>()(key.second) << 1);
    }
};

class ModulationController{
private:
    ModulatorID nextID_ = 0 ;
    std::unordered_map<ModulatorID, std::unique_ptr<BaseModulator>> modulators_ ;
    std::unordered_map<ModulatorKey, ModulatorID, ModulatorKeyHash> typeLookup_;
    std::unordered_map<std::string, ModulatorID> nameLookup_ ;
    std::unordered_map<ModulatorType, int> typeInstanceCount_ ;

public:
    template <ModulatorType T, typename... Args>
    ModulatorID create(std::string name, Args&&... args){
        static_assert(std::is_base_of<BaseModulator, ModulatorType_t<T>>::value, "Must be derived from Modulator");

        int idx = ++typeInstanceCount_[T] ;
        ModulatorID id = nextID_++ ;
        
        auto m = std::make_unique<ModulatorType_t<T>>(std::forward<Args>(args)...);
        modulators_[id] = std::move(m);

        typeLookup_[{T,idx}] = id ;
        nameLookup_[name] = id ;
        
        return id ;
    }

    BaseModulator* getRaw(ModulatorID id){
        auto it = modulators_.find(id);
        if ( it == modulators_.end() ) return nullptr ;
        return it->second.get() ;
    }

    template<typename T>
    T* get(ModulatorID id){
        BaseModulator* m = getRaw(id);
        if (!m) return nullptr ;

        if (m->getType() == T::staticType){
            return static_cast<T*>(m);
        }

        return nullptr ;
    }

    template<typename T>
    T* getByTypeInstance(int idx){
        ModulatorKey key = { T::staticType, idx};
        auto it = typeLookup_.find(key);
        if (it == typeLookup_.end()) return nullptr ;
        return get<T>(it->second) ;
    }

    BaseModulator* getByName(const std::string& name){
        auto it = nameLookup_.find(name);
        if ( it == nameLookup_.end() ) return nullptr ;
        return modulators_[it->second].get() ;
    }

    void tick(float dt){
        for (auto it = modulators_.begin(); it != modulators_.end(); ++it){
            MidiEventHandler* h = dynamic_cast<MidiEventHandler*>(it->second.get());
            if (h) h->tick(dt);
        }
    }

    void reset(){
        nextID_ = 0 ;
        modulators_.clear() ;
        typeLookup_.clear() ;
        nameLookup_.clear() ;
        typeInstanceCount_.clear() ;
    }
};

#endif // __MODULATION_CONTROLLER_HPP_