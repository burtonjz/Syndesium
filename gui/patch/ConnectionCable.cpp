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

#include "patch/ConnectionCable.hpp"
#include "patch/SocketWidget.hpp"
#include "core/Theme.hpp"

#include <QPainter>
#include <QGraphicsScene>
#include <QGraphicsSceneMouseEvent>
#include <cmath>

ConnectionCable::ConnectionCable(SocketWidget* fromSocket, SocketWidget* toSocket): 
    QGraphicsPathItem(nullptr), 
    fromSocket_(fromSocket), 
    toSocket_(toSocket)
{
    setZValue(-0.1); // above socket, below container
    setPen(QPen(getCableColor(), 3, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
    if (!toSocket) endpoint_ = fromSocket_->getConnectionPoint() + QPoint(5,5); // set initial endpoint on creation, offset slightly so visible
    updatePath();
}

SocketWidget* ConnectionCable::getInboundSocket() const {
    if ( fromSocket_ && fromSocket_->isInput() ) return fromSocket_ ;
    if ( toSocket_ && toSocket_->isInput() ) return toSocket_ ;
    return nullptr ;
}

SocketWidget* ConnectionCable::getOutboundSocket() const {
    if ( fromSocket_ && fromSocket_->isOutput() ) return fromSocket_ ;
    if ( toSocket_ && toSocket_->isOutput() ) return toSocket_ ;
    return nullptr ;
}

void ConnectionCable::setFromSocket(SocketWidget* socket){
    fromSocket_ = socket;
    updatePath();
}

void ConnectionCable::setToSocket(SocketWidget* socket){
    toSocket_ = socket;
    updatePath();
}

void ConnectionCable::setEndpoint(const QPointF& point){
    endpoint_  = point;
    updatePath();
}

bool ConnectionCable::isCompatible(SocketWidget* socket) const {
    if (!fromSocket_ || !socket) return false ;
    
    // Can't connect socket to itself
    if (fromSocket_ == socket) return false ;
    
    // Can't connect sockets from the same container widget
    if (fromSocket_->getParent() == socket->getParent()) return false ;
    
    // Must be compatible types
    SocketType fromType = fromSocket_->getType();
    SocketType toType = socket->getType();

    switch ( fromType ){
    case SocketType::MidiInput:
        return toType == SocketType::MidiOutput ;
    case SocketType::MidiOutput:
        return toType == SocketType::MidiInput ;
    case SocketType::ModulationInput:
        return toType == SocketType::ModulationOutput ;
    case SocketType::ModulationOutput:
        return toType == SocketType::ModulationInput ;
    case SocketType::SignalInput:
        return toType == SocketType::SignalOutput ;
    case SocketType::SignalOutput:
        return toType == SocketType::SignalInput ;
    default:
        return false ;
    }
}

bool ConnectionCable::involvesWidget(SocketContainerWidget* widget) const {
    return ( fromSocket_ && fromSocket_->getParent() == widget ) ||
           ( toSocket_   &&   toSocket_->getParent() == widget ) ;
}

bool ConnectionCable::involvesSocket(SocketWidget* socket) const {
    return ( fromSocket_ && fromSocket_ == socket ) ||
           ( toSocket_ && toSocket_ == socket ) ;
}

void ConnectionCable::updatePath(){
    if (!fromSocket_) return ;
    
    QPointF startPoint = fromSocket_->getConnectionPoint();
    QPointF endPoint ;
    
    if ( toSocket_ ) {
        endPoint = toSocket_->getConnectionPoint();
    } else {
        endPoint = endpoint_ ;
    }
    
    QPainterPath newPath = createBezierPath(startPoint, endPoint);
    setPath(newPath);
    
    // Update pen color
    setPen(QPen(getCableColor(), 3, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
}

QPainterPath ConnectionCable::createBezierPath(const QPointF& start, const QPointF& end){
    QPainterPath path ;
    path.moveTo(start) ;
    
    // Calculate control points for a nice curved cable
    qreal dx = end.x() - start.x();
    qreal dy = end.y() - start.y();
    
    // Make curves more pronounced for longer distances
    qreal controlOffset = std::max(50.0, std::abs(dx) * 0.5);
    
    QPointF control1, control2 ;
    
    if ( dx >= 0 ){
        control1 = start + QPointF(controlOffset, 0);
        control2 = end - QPointF(controlOffset, 0);
    } else {
        control1 = start - QPointF(controlOffset, 0);
        control2 = end + QPointF(controlOffset, 0);
    }

    path.cubicTo(control1, control2, end);
    return path;
}

QColor ConnectionCable::getCableColor() const
{
    if (!fromSocket_) return Qt::gray;
    
    switch (fromSocket_->getType()) {
        case SocketType::ModulationInput:
        case SocketType::ModulationOutput:
            return Theme::CABLE_MODULATION ;
        case SocketType::SignalInput:
        case SocketType::SignalOutput:
            return Theme::CABLE_AUDIO ;
        case SocketType::MidiInput:
        case SocketType::MidiOutput:
            return Theme::CABLE_MIDI ;
        default:
            return Qt::gray ;
    }
}

void ConnectionCable::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget){
    painter->setRenderHint(QPainter::Antialiasing);
    
    // Draw a subtle shadow first
    QPen shadowPen = pen();
    shadowPen.setColor(Theme::CABLE_SHADOW);
    shadowPen.setWidth(pen().width() + 2);
    painter->setPen(shadowPen);
    painter->drawPath(path().translated(1, 1));
    
    // Draw the main cable
    QGraphicsPathItem::paint(painter, option, widget);
}