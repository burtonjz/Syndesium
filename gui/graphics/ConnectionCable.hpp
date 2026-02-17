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

#ifndef CONNECTION_CABLE_HPP_
#define CONNECTION_CABLE_HPP_

#include <QGraphicsPathItem>

#include "widgets/SocketWidget.hpp"
#include "requests/ConnectionRequest.hpp"

class ConnectionCable : public QGraphicsPathItem {
private:
    SocketWidget* fromSocket_ ;
    SocketWidget* toSocket_ ;
    QPointF endpoint_ ;

public:
    ConnectionCable(SocketWidget* fromSocket, SocketWidget* toSocket = nullptr);

    bool operator==(const ConnectionCable& other) const ;
    bool operator==(const ConnectionRequest& req) const ;

    SocketWidget* getFromSocket() const { return fromSocket_ ; }
    SocketWidget* getToSocket() const { return toSocket_ ; }
    
    SocketWidget* getOutboundSocket() const ;
    SocketWidget* getInboundSocket() const ;

    void setFromSocket(SocketWidget* socket);
    void setToSocket(SocketWidget* socket);

    void setEndpoint(const QPointF& end);

    bool isComplete() const { return fromSocket_ && toSocket_ ; }
    bool isCompatible(SocketWidget* socket) const ;

    bool involvesWidget(GraphNode* widget) const ;
    bool involvesSocket(SocketWidget* socket) const ;

    void updatePath();

    ConnectionRequest toConnectionRequest() const ;
    QString toText() const ;

protected:
    void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget *widget ) override ;

private:
    QColor getCableColor() const ;

    QPainterPath createBezierPath(const QPointF& start, const QPointF& end); // legacy

    /**
     * @brief draw the adapted bezier path
     */
    QPainterPath createAdaptiveBezierPath(const QPointF& start, const QPointF& end);

    void drawCableArrow(QPainterPath& path, qreal atPercent);
    QPointF getSocketDirectionVector(SocketWidget* sock);
    QPointF normalizePoint(const QPointF& p) const ;

};

#endif // CONNECTION_CABLE_HPP_