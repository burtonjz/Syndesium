#include "widgets/ComponentWidget.hpp"
#include "meta/ComponentRegistry.hpp"
#include "patch/SocketWidget.hpp"

#include <QGraphicsSceneMouseEvent>
#include <vector>

ComponentWidget::ComponentWidget(int id, ComponentType type, QGraphicsItem* parent): 
    SocketContainerWidget(QString::fromStdString(ComponentRegistry::getComponentDescriptor(type).name), parent),
    componentId_(id),
    descriptor_(ComponentRegistry::getComponentDescriptor(type))
{
    std::vector<SocketSpec> specs ;

    // create sockets from descriptor
    for ( const ParameterType& p : descriptor_.modulatableParameters){
        specs.push_back({SocketType::ModulationInput, QString::fromStdString(parameter2String(p))});
    }

    for (int i = 0; i < descriptor_.numAudioInputs; ++i){
        specs.push_back({SocketType::SignalInput, QString("Audio Input %1").arg(i+1)});
    }

    for (int i = 0; i < descriptor_.numMidiInputs; ++i){
        specs.push_back({SocketType::MidiInput, QString("MIDI Input %1").arg(i+1)});
    }

    for (int i = 0; i < descriptor_.numAudioOutputs; ++i){
        specs.push_back({SocketType::SignalOutput, QString("Audio Output %1").arg(i+1)});
    }

    for (int i = 0; i < descriptor_.numMidiOutputs; ++i){
        specs.push_back({SocketType::MidiOutput, QString("MIDI Output %1").arg(i+1)});
    }

    if ( descriptor_.type.isModulator()){
        specs.push_back({SocketType::ModulationOutput, QString("Modulation Output")});
    }

    createSockets(specs);

}
