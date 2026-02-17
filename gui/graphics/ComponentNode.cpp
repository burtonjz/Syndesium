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

#include "graphics/ComponentNode.hpp"
#include "widgets/SocketWidget.hpp"
#include "types/ParameterType.hpp"

#include <QGraphicsSceneMouseEvent>
#include <vector>

ComponentNode::ComponentNode(ComponentModel* model, QGraphicsItem* parent): 
    GraphNode(QString::fromStdString(model->getDescriptor().name), parent),
    model_(model),
    specs_()
{
    auto d = model_->getDescriptor();
    
    // create sockets from descriptor
    for ( const ParameterType& p : d.modulatableParameters){
        std::string name = GET_PARAMETER_TRAIT_MEMBER(p,name);
        specs_.push_back({SocketType::ModulationInbound, QString::fromStdString(name)});
    }

    for (int i = 0; i < d.numAudioInputs; ++i){
        specs_.push_back({SocketType::SignalInbound, QString("Audio Input %1").arg(i+1), i});
    }

    for (int i = 0; i < d.numMidiInputs; ++i){
        specs_.push_back({SocketType::MidiInbound, QString("MIDI Input %1").arg(i+1)});
    }

    for (int i = 0; i < d.numAudioOutputs; ++i){
        specs_.push_back({SocketType::SignalOutbound, QString("Audio Output %1").arg(i+1), i});
    }

    for (int i = 0; i < d.numMidiOutputs; ++i){
        specs_.push_back({SocketType::MidiOutbound, QString("MIDI Output %1").arg(i+1)});
    }

    if ( d.isModulator()){
        specs_.push_back({SocketType::ModulationOutbound, QString("Modulation Output")});
    }

    createSockets(specs_);

}

ComponentModel* ComponentNode::getModel() const {
    return model_ ;
}
