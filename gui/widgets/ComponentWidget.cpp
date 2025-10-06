/*
 * Copyright (C) 2025 Jared Burton
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program. If not, see <https://www.gnu.org/licenses/>.
 */

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

    if ( descriptor_.isModulator()){
        specs.push_back({SocketType::ModulationOutput, QString("Modulation Output")});
    }

    createSockets(specs);

}
