#ifndef STATEMANAGER_HPP
#define STATEMANAGER_HPP

#include <QObject>

class StateManager : public QObject
{
    Q_OBJECT

private:
    bool setupAudio_ = false ;
    bool setupMidi_  = false ;
    bool running_    = false ;

public:
    explicit StateManager(QObject *parent = nullptr);

    void setSetupAudioComplete(bool v);
    void setSetupMidiComplete(bool v);
    bool isRunning() const ;
    void setRunning(bool v);

private:
    void checkSetupConditions();

signals:
    void setupCompleted();

};

#endif // STATEMANAGER_HPP
