#include "core/ComponentFactory.hpp"
#include "configs/ComponentConfig.hpp"

#include "components/Oscillator.hpp"
#include "components/PolyOscillator.hpp" 
#include "components/ADSREnvelope.hpp" 
#include "components/LinearFader.hpp"
#include "components/MidiFilter.hpp" 

#define HANDLE_CREATE_COMPONENT(Type) \
    case ComponentType::Type: \
        return store_->create<ComponentType::Type>(name,  j.get<Type##Config>());

ComponentFactory::ComponentFactory(ComponentManager* store):
    store_(store)
{}

ComponentId ComponentFactory::createFromJson(ComponentType type, const std::string& name, const json& j ){
    switch(type){
        HANDLE_CREATE_COMPONENT(Oscillator)
        HANDLE_CREATE_COMPONENT(PolyOscillator)
        HANDLE_CREATE_COMPONENT(LinearFader)
        HANDLE_CREATE_COMPONENT(ADSREnvelope)
        HANDLE_CREATE_COMPONENT(MidiFilter)
    default:
        throw std::runtime_error("invalid component requested.");
    }
}