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

#include "widgets/SocketContainerWidget.hpp"
#include "patch/SocketWidget.hpp"

#include <QGraphicsSceneMouseEvent>

SocketContainerWidget::SocketContainerWidget(QString name, QGraphicsItem* parent): 
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
    titleText_->setDefaultTextColor(COMPONENT_TEXT_COLOR);
    titleText_->setPos(COMPONENT_TEXT_PADDING,COMPONENT_TEXT_PADDING);
    titleText_->setTextWidth(COMPONENT_WIDTH - COMPONENT_TEXT_PADDING * 2);
}

QRectF SocketContainerWidget::boundingRect() const {
    qreal delta = HIGHLIGHT_BUFFER + HIGHLIGHT_WIDTH ;
    return QRectF(0, 0, COMPONENT_WIDTH, COMPONENT_HEIGHT)
        .adjusted(-delta, -delta, delta, delta);
}

void SocketContainerWidget::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget){
    Q_UNUSED(option)
    Q_UNUSED(widget)

    // draw background
    QRectF baseRect(0, 0,COMPONENT_WIDTH, COMPONENT_HEIGHT);
    painter->setBrush(COMPONENT_BACKGROUND_COLOR);
    painter->setPen(QPen(Qt::white,COMPONENT_BORDER_WIDTH));
    painter->drawRoundedRect(baseRect, COMPONENT_ROUNDED_RADIUS, COMPONENT_ROUNDED_RADIUS);

    // when selected, draw an indicator around the object
    if (isSelected()){
        painter->setPen(QPen(Qt::yellow, HIGHLIGHT_WIDTH, Qt::DashLine));
        painter->setBrush(Qt::NoBrush);
        painter->drawRoundedRect(
            baseRect.adjusted(-HIGHLIGHT_BUFFER,-HIGHLIGHT_BUFFER,HIGHLIGHT_BUFFER,HIGHLIGHT_BUFFER),
            COMPONENT_ROUNDED_RADIUS,COMPONENT_ROUNDED_RADIUS
        );
    }
}

void SocketContainerWidget::createSockets(std::initializer_list<SocketSpec> specs ){
    for ( const auto& s : specs ){
        SocketWidget* socket = new SocketWidget(s, this);
        sockets_.append(socket);
    }

    layoutSockets();
}

void SocketContainerWidget::createSockets(std::vector<SocketSpec> specs){
    for ( const auto& s : specs ){
        SocketWidget* socket = new SocketWidget(s, this);
        sockets_.append(socket);
    }

    layoutSockets();
}

void SocketContainerWidget::layoutSockets(){
    QList<SocketWidget*> leftSockets ; // audio / midi inputs
    QList<SocketWidget*> rightSockets ; // audio / midi outputs
    QList<SocketWidget*> bottomSockets ; // modulatable inputs
    QList<SocketWidget*> topSockets ; // modulatable outputs

    for (SocketWidget* socket : sockets_){
        switch(socket->getType()){
        case SocketType::MidiInput:
        case SocketType::SignalInput:
            leftSockets.append(socket);
            break ;
        case SocketType::MidiOutput:
        case SocketType::SignalOutput:
            rightSockets.append(socket);
            break ;
        case SocketType::ModulationInput:
            bottomSockets.append(socket);
            break ;
        case SocketType::ModulationOutput:
            topSockets.append(socket);
            break ;
        default:
            break ;
        }
    }

    qreal startY = 25 ; // below the title
    // left
    for ( int i = 0; i < leftSockets.size(); ++i ){
        leftSockets[i]->setPos(-6, startY + i * SOCKET_SPACING);
    }

    // right
    for ( int i = 0; i < rightSockets.size(); ++i ){
        rightSockets[i]->setPos(COMPONENT_WIDTH + 6, startY + i * SOCKET_SPACING);
    }

    qreal startX = 4 ;
    // bottom
    for ( int i = 0; i < bottomSockets.size(); ++i ){
        bottomSockets[i]->setPos(startX + i * SOCKET_SPACING, COMPONENT_HEIGHT + 6 );
    }

    // top
    for ( int i = 0; i < topSockets.size(); ++i ){
        topSockets[i]->setPos(COMPONENT_WIDTH - 6 - i * SOCKET_SPACING, -6);
    }
}

void SocketContainerWidget::mousePressEvent(QGraphicsSceneMouseEvent *event){
    if (event->button() == Qt::LeftButton) {
        isDragging_ = true;
        dragStartPos_ = event->pos();
    }
    QGraphicsObject::mousePressEvent(event);
}

void SocketContainerWidget::mouseMoveEvent(QGraphicsSceneMouseEvent *event){
    if (isDragging_) {
        // Handle dragging the module
        QGraphicsObject::mouseMoveEvent(event);
    }
}

void SocketContainerWidget::mouseReleaseEvent(QGraphicsSceneMouseEvent *event){
    if (event->button() == Qt::LeftButton) {
        isDragging_ = false;
    }
    QGraphicsObject::mouseReleaseEvent(event);
}

QVariant SocketContainerWidget::itemChange(GraphicsItemChange change, const QVariant& value ){
    if ( change == ItemPositionChange || change == ItemPositionHasChanged ){
        emit positionChanged() ;
    }
    return QGraphicsObject::itemChange(change, value);
}