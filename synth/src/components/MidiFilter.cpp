#include "components/MidiFilter.hpp"
#include "midi/MidiEventHandler.hpp"
#include "params/ParameterMap.hpp"
#include "types/ParameterType.hpp"

MidiFilter::MidiFilter(ComponentId id, MidiFilterConfig cfg):
    BaseComponent(id, ComponentType::MidiFilter)
{
    parameters_->add<ParameterType::MIN_VALUE>(cfg.min_value,false);
    parameters_->add<ParameterType::MAX_VALUE>(cfg.max_value,false);

}

void MidiFilter::onKeyPressed(const ActiveNote* note, bool rePressed){
    if ( 
        note &&
        note->note.getMidiNote() >= parameters_->getValue<ParameterType::MIN_VALUE>() &&
        note->note.getMidiNote() <= parameters_->getValue<ParameterType::MAX_VALUE>()
    ){
        MidiEventHandler::onKeyPressed(note, rePressed);
    }
}

void MidiFilter::onKeyReleased(ActiveNote anote){
    if ( 
        anote.note.getMidiNote() >= parameters_->getValue<ParameterType::MIN_VALUE>() &&
        anote.note.getMidiNote() <= parameters_->getValue<ParameterType::MAX_VALUE>()
    ){
        MidiEventHandler::onKeyReleased(anote);
    }
}
