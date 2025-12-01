#include "components/MidiFilter.hpp"
#include "midi/MidiEventHandler.hpp"
#include "params/ParameterMap.hpp"
#include "types/ParameterType.hpp"

MidiFilter::MidiFilter(ComponentId id, MidiFilterConfig cfg):
    BaseComponent(id, ComponentType::MidiFilter)
{
    parameters_->add<ParameterType::MIN_VALUE>(cfg.min_value,false);
    parameters_->add<ParameterType::MAX_VALUE>(cfg.max_value,false);
    parameters_->finalizeParameters();

}

void MidiFilter::onKeyPressed(const ActiveNote* note, bool rePressed){
    if ( 
        note &&
        note->note.getMidiNote() >= parameters_->getParameter<ParameterType::MIN_VALUE>()->getValue() &&
        note->note.getMidiNote() <= parameters_->getParameter<ParameterType::MAX_VALUE>()->getValue()
    ){
        MidiEventHandler::onKeyPressed(note, rePressed);
    }
}

void MidiFilter::onKeyReleased(ActiveNote anote){
    if ( 
        anote.note.getMidiNote() >= parameters_->getParameter<ParameterType::MIN_VALUE>()->getValue() &&
        anote.note.getMidiNote() <= parameters_->getParameter<ParameterType::MAX_VALUE>()->getValue()
    ){
        MidiEventHandler::onKeyReleased(anote);
    }
}
