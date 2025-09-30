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

#ifndef __GUI_SOCKET_WIDGET_HPP_
#define __GUI_SOCKET_WIDGET_HPP_

#include <QGraphicsObject>
#include <QWidget>
#include <QPainter>
#include <QStyleOptionGraphicsItem>

#include "types/SocketType.hpp"

class SocketContainerWidget ; // forward declaration

struct SocketSpec {
    SocketType type ;
    QString name ;
};

class SocketWidget : public QGraphicsObject {
    Q_OBJECT

private:
    SocketSpec spec_ ;
    SocketContainerWidget* parent_ ;
    bool isHovered_ = false ;
    bool isDragging_ = false ;
    QColor getSocketColor() const ;

    static constexpr qreal SOCKET_RADIUS = 6.0 ;

public:
    SocketWidget(SocketSpec spec, SocketContainerWidget* parent = nullptr);

    // QGraphicsItem interface
    QRectF boundingRect() const override ;
    void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) override ;

    // Getters
    SocketType getType() const { return spec_.type ; } 
    SocketContainerWidget* getParent() const { return parent_ ; }
    const QString& getName() const { return spec_.name ; }
    bool isOutput() const ;
    bool isInput() const ;
    QPointF getConnectionPoint() const ;

protected:
    void mousePressEvent(QGraphicsSceneMouseEvent* event) override ;
    void mouseMoveEvent(QGraphicsSceneMouseEvent* event) override ;
    void mouseReleaseEvent(QGraphicsSceneMouseEvent* event) override ;
    void hoverEnterEvent(QGraphicsSceneHoverEvent* event) override ;
    void hoverLeaveEvent(QGraphicsSceneHoverEvent* event) override ;

signals:
    void connectionStarted(SocketWidget* socket);
    void connectionDragging(SocketWidget* socket, QPointF scenePos);
    void connectionEnded(SocketWidget* socket, QPointF scenePos);

};

#endif // __GUI_SOCKET_WIDGET_HPP_