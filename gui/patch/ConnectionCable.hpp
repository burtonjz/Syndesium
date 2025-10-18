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

#ifndef __GUI_CONNECTION_CABLE_HPP_
#define __GUI_CONNECTION_CABLE_HPP_

#include <QGraphicsPathItem>

#include "patch/SocketWidget.hpp"

class ConnectionCable : public QGraphicsPathItem {
private:
    SocketWidget* fromSocket_ ;
    SocketWidget* toSocket_ ;
    QPointF endpoint_ ;

public:
    ConnectionCable(SocketWidget* fromSocket, SocketWidget* toSocket = nullptr);

    SocketWidget* getFromSocket() const { return fromSocket_ ; }
    SocketWidget* getToSocket() const { return toSocket_ ; }
    
    SocketWidget* getOutboundSocket() const ;
    SocketWidget* getInboundSocket() const ;

    void setFromSocket(SocketWidget* socket);
    void setToSocket(SocketWidget* socket);

    void setEndpoint(const QPointF& end);

    bool isComplete() const { return fromSocket_ && toSocket_ ; }
    bool isCompatible(SocketWidget* socket) const ;

    bool involvesWidget(SocketContainerWidget* widget) const ;

    void updatePath();

protected:
    void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget *widget ) override ;

private:
    QPainterPath createBezierPath(const QPointF& start, const QPointF& end);
    QColor getCableColor() const ;

};

#endif // __GUI_CONNECTION_CABLE_HPP_