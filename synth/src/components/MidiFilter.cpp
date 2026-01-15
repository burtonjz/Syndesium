#include "components/MidiFilter.hpp"
#include "midi/MidiEventHandler.hpp"
#include "params/ParameterMap.hpp"
#include "types/ParameterType.hpp"

MidiFilter::MidiFilter(ComponentId id, MidiFilterConfig cfg):
    BaseComponent(id, ComponentType::MidiFilter)
{
    parameters_->addCollection<ParameterType::MIDI_VALUE>({cfg.min, cfg.max});
}

void MidiFilter::onKeyPressed(const ActiveNote* note, bool rePressed){
    if ( !note ) return ;
    
    if ( passNote(note->note.getMidiNote()) ){
        MidiEventHandler::onKeyPressed(note, rePressed);
    } 
}

void MidiFilter::onKeyReleased(ActiveNote anote){
    if ( passNote(anote.note.getMidiNote()) ){
        MidiEventHandler::onKeyReleased(anote);
    }
}

bool MidiFilter::passNote(uint8_t midi) const {
    auto c = parameters_->getCollection<ParameterType::MIDI_VALUE>();
    bool passNote = true ;
    for ( size_t i = 0 ; i < c->size() - 1 ; i += 2 ){
        passNote = passNote 
            && midi >= c->getValue(i)
            && midi <= c->getValue(i+1);
        if ( !passNote ) break ;
    }
    return passNote ;
}