#include "StateManager.hpp"

StateManager::StateManager(QObject *parent)
    : QObject{parent} {

}

void StateManager::setSetupAudioComplete(bool v){
    setupAudio_ = v ;
    checkSetupConditions() ;
}

void StateManager::setSetupMidiComplete(bool v){
    setupMidi_ = v ;
    checkSetupConditions() ;
}

bool StateManager::isRunning() const  {
    return running_ ;
}

void StateManager::setRunning(bool v ){
    running_ = v ;
}

void StateManager::checkSetupConditions(){
    if ( setupAudio_ && setupMidi_ ){
        emit setupCompleted() ;
    }
}

