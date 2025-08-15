#include "widgets/ModuleWidget.hpp"
#include "meta/ModuleRegistry.hpp"
#include "patch/SocketWidget.hpp"

#include <QGraphicsSceneMouseEvent>
#include <vector>

ModuleWidget::ModuleWidget(int id, ModuleType type, QGraphicsItem* parent): 
    SocketContainerWidget(QString::fromStdString(ModuleRegistry::getModuleDescriptor(type).name), parent),
    moduleId_(id),
    descriptor_(ModuleRegistry::getModuleDescriptor(type))
{
    std::vector<SocketSpec> specs ;

    // create sockets from descriptor
    for ( const ParameterType& p : descriptor_.modulatableParameters){
        specs.push_back({SocketType::ModulationInput, QString::fromStdString(parameter2String(p))});
    }

    for (int i = 0; i < descriptor_.numAudioInputs; ++i){
        specs.push_back({SocketType::SignalInput, QString("AUDIO IN %1").arg(i+1)});
    }

    for (int i = 0; i < descriptor_.numMidiInputs; ++i){
        specs.push_back({SocketType::MidiInput, QString("MIDI IN %1").arg(i+1)});
    }

    for (int i = 0; i < descriptor_.numAudioOutputs; ++i){
        specs.push_back({SocketType::SignalOutput, QString("AUDIO OUT %1").arg(i+1)});
    }

    for (int i = 0; i < descriptor_.numMidiOutputs; ++i){
        specs.push_back({SocketType::MidiOutput, QString("MIDI OUT %1").arg(i+1)});
    }

    createSockets(specs);

}
