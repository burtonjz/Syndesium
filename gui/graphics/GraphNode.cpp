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

#include "graphics/GraphNode.hpp"
#include "widgets/SocketWidget.hpp"
#include "app/Theme.hpp"

#include <QGraphicsSceneMouseEvent>
#include <QGraphicsScene>
#include <QPoint>

GraphNode::GraphNode(QString name, QGraphicsItem* parent): 
    QGraphicsObject(parent),
    name_(name)
{
    // configure widget
    setFlag(QGraphicsItem::ItemIsMovable, true);
    setFlag(QGraphicsItem::ItemIsSelectable, true);
    setFlag(QGraphicsItem::ItemSendsGeometryChanges, true);
    setAcceptedMouseButtons(Qt::LeftButton | Qt::RightButton);
    setAcceptHoverEvents(true);

    titleText_ = new QGraphicsTextItem(name_, this);
    titleText_->setAcceptedMouseButtons(Qt::NoButton);

    titleText_->setDefaultTextColor(Theme::Theme::COMPONENT_TEXT);
    titleText_->setPos(Theme::COMPONENT_TEXT_PADDING,Theme::COMPONENT_TEXT_PADDING);
    titleText_->setTextWidth(Theme::COMPONENT_WIDTH - Theme::COMPONENT_TEXT_PADDING * 2);

}

GraphNode::~GraphNode(){
    for ( auto socket : sockets_ ){
        socket->deleteLater();
    }
}

QRectF GraphNode::boundingRect() const {
    qreal delta = Theme::COMPONENT_HIGHLIGHT_BUFFER + Theme::COMPONENT_HIGHLIGHT_WIDTH ;
    return QRectF(0, 0, Theme::COMPONENT_WIDTH, Theme::COMPONENT_HEIGHT)
        .adjusted(-delta, -delta, delta, delta);
}

void GraphNode::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget){
    Q_UNUSED(option)
    Q_UNUSED(widget)

    // draw background
    QRectF baseRect(0, 0,Theme::COMPONENT_WIDTH, Theme::COMPONENT_HEIGHT);
    painter->setBrush(Theme::Theme::COMPONENT_BACKGROUND);
    painter->setPen(QPen(Theme::Theme::COMPONENT_BORDER,Theme::COMPONENT_BORDER_WIDTH));
    painter->drawRoundedRect(baseRect, Theme::COMPONENT_ROUNDED_RADIUS, Theme::COMPONENT_ROUNDED_RADIUS);

    // when selected, draw an indicator around the object
    if (isSelected()){
        painter->setPen(QPen(Theme::Theme::COMPONENT_BORDER_SELECTED, Theme::COMPONENT_HIGHLIGHT_WIDTH, Qt::SolidLine));
        painter->setBrush(Qt::NoBrush);
        painter->drawRoundedRect(
            baseRect.adjusted(-Theme::COMPONENT_HIGHLIGHT_BUFFER,-Theme::COMPONENT_HIGHLIGHT_BUFFER,Theme::COMPONENT_HIGHLIGHT_BUFFER,Theme::COMPONENT_HIGHLIGHT_BUFFER),
            Theme::COMPONENT_ROUNDED_RADIUS,Theme::COMPONENT_ROUNDED_RADIUS
        );
    }
}

void GraphNode::createSockets(std::initializer_list<SocketSpec> specs ){
    for ( const auto& s : specs ){
        SocketWidget* socket = new SocketWidget(s, this);
        sockets_.push_back(socket);
    }

    layoutSockets();
    positionSockets();
}

void GraphNode::createSockets(std::vector<SocketSpec> specs){
    for ( const auto& s : specs ){
        SocketWidget* socket = new SocketWidget(s, this);
        sockets_.push_back(socket);
    }

    layoutSockets();
    positionSockets();
}

void GraphNode::hide(){
    QGraphicsItem::hide();
    for ( auto& s : getSockets() ){
        s->hide();
    }
}

void GraphNode::show(){
    QGraphicsItem::show();
    for ( auto& s : getSockets() ){
        s->show();
    }
}

void GraphNode::addToScene(QGraphicsScene* scene){
    scene->addItem(this);
    for ( auto* socket : sockets_ ) scene->addItem(socket);
}

void GraphNode::layoutSockets(){
    leftSockets_.clear();
    rightSockets_.clear();
    topSockets_.clear();
    bottomSockets_.clear();

    for (SocketWidget* socket : sockets_){
        switch(socket->getSpec().type){
        case SocketType::MidiInbound:
        case SocketType::SignalInbound:
            leftSockets_.push_back(socket);
            break ;
        case SocketType::MidiOutbound:
        case SocketType::SignalOutbound:
            rightSockets_.push_back(socket);
            break ;
        case SocketType::ModulationInbound:
            bottomSockets_.push_back(socket);
            break ;
        case SocketType::ModulationOutbound:
            topSockets_.push_back(socket);
            break ;
        default:
            break ;
        }
    }
}

void GraphNode::positionSockets(QPointF newPos){
    QPointF scenePos = newPos;

    qreal startY = 25 ; // below the title
    // left
    for ( int i = 0; i < leftSockets_.size(); ++i ){
        leftSockets_[i]->setPos(scenePos + QPointF(-6, startY + i * Theme::SOCKET_WIDGET_SPACING));
    }

    // right
    for ( int i = 0; i < rightSockets_.size(); ++i ){
        rightSockets_[i]->setPos(scenePos + QPointF(Theme::COMPONENT_WIDTH + 6, startY + i * Theme::SOCKET_WIDGET_SPACING));
    }

    qreal startX = 4 ;
    // bottom
    for ( int i = 0; i < bottomSockets_.size(); ++i ){
        bottomSockets_[i]->setPos(scenePos + QPointF(startX + i * Theme::SOCKET_WIDGET_SPACING, Theme::COMPONENT_HEIGHT + 6 ));
    }

    // top
    for ( int i = 0; i < topSockets_.size(); ++i ){
        topSockets_[i]->setPos(scenePos + QPointF(Theme::COMPONENT_WIDTH - 6 - i * Theme::SOCKET_WIDGET_SPACING, -6));
    }
}

QVariant GraphNode::itemChange(GraphicsItemChange change, const QVariant& value ){
    if ( change == ItemPositionChange ){
        positionSockets(value.toPointF());
        emit positionChanged() ;
    }

    // if the item is selected, we need to inform graph panel to move it to the top
    if ( change == ItemSelectedChange ){
        if ( value.toBool()) emit needsZUpdate();
    }
    return QGraphicsObject::itemChange(change, value);
}