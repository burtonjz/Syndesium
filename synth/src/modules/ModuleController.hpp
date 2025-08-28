#ifndef __MODULE_CONTROLLER_HPP_
#define __MODULE_CONTROLLER_HPP_

#include "modules/SignalChain.hpp"
#include "modules/BaseModule.hpp"
#include "modules/Oscillator.hpp"
#include "modules/PolyOscillator.hpp"
#include "configs/ModuleConfig.hpp"

#include "types/ModuleType.hpp"
#include "types/Waveform.hpp"

#include "config/Config.hpp"

#include <cstddef>
#include <memory>
#include <stdexcept>
#include <utility>
#include <type_traits>
#include <unordered_map>

using ModuleID = int ;
using ModuleKey = std::pair<ModuleType, int>;

#define HANDLE_CREATE_MODULE(Type) \
    case ModuleType::Type: \
        return create<ModuleType::Type>(name,  j.get<Type##Config>());

struct ModuleKeyHash {
    ModuleID operator()(const ModuleKey& key) const {
        return std::hash<int>()(static_cast<int>(key.first)) ^ (std::hash<int>()(key.second) << 1);
    }
};

class ModuleController{
private:
    static constexpr ModuleID HARDWARE_AUDIO_INPUT_ID = -1 ;
    static constexpr ModuleID HARDWARE_AUDIO_OUTPUT_ID = -2 ;

    ModuleID nextID_ = 0 ;
    std::unordered_map<ModuleID, std::unique_ptr<BaseModule>> Modules_ ;
    std::unordered_map<ModuleKey, ModuleID, ModuleKeyHash> typeLookup_;
    std::unordered_map<std::string, ModuleID> nameLookup_ ;
    std::unordered_map<ModuleType, int> typeInstanceCount_ ;
    SignalChain signalChain_ ;

public:
    template <ModuleType T>
    ModuleID create(std::string name, ModuleConfig_t<T> cfg){
        static_assert(std::is_base_of<BaseModule, ModuleType_t<T>>::value, "Must be derived from Module");

        Config::load();
        double sampleRate = Config::get<double>("audio.sample_rate").value();
        size_t bufferSize = Config::get<size_t>("audio.buffer_size").value();

        int idx = ++typeInstanceCount_[T] ;
        ModuleID id = nextID_++ ;
        
        auto m = std::make_unique<ModuleType_t<T>>(sampleRate, bufferSize, cfg);
        Modules_[id] = std::move(m);

        typeLookup_[{T,idx}] = id ;
        nameLookup_[name] = id ;
        
        return id ;
    }

    ModuleID dispatchFromJson(ModuleType type, const std::string& name, const json& j){
        switch (type){
            HANDLE_CREATE_MODULE(Oscillator)
            HANDLE_CREATE_MODULE(PolyOscillator)
            default:
                throw std::invalid_argument("Unsupported ModuleType");
        }
    }

    bool isHardwareID(ModuleID id) const {
        return id == HARDWARE_AUDIO_INPUT_ID || id == HARDWARE_AUDIO_OUTPUT_ID ;
    }

    ModuleID getAudioInputID() const { return HARDWARE_AUDIO_INPUT_ID ; }
    ModuleID getAudioOutputID() const { return HARDWARE_AUDIO_OUTPUT_ID ; }

    BaseModule* getRaw(ModuleID id){
        auto it = Modules_.find(id);
        if ( it == Modules_.end() ) return nullptr ;
        return it->second.get() ;
    }

    template<ModuleType T>
    ModuleType_t<T>* get(ModuleID id){
        BaseModule* m = getRaw(id);
        if (!m) return nullptr ;

        if (m->getType() == T ){
            return static_cast<ModuleType_t<T>* >(m);
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

    BaseModule* getByName(const std::string& name){
        auto it = nameLookup_.find(name);
        if ( it == nameLookup_.end() ) return nullptr ;
        return Modules_[it->second].get() ;
    }

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