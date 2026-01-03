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

#ifndef __GUI_CONNECTION_MANAGER_HPP_
#define __GUI_CONNECTION_MANAGER_HPP_

#include <QObject>
#include <QGraphicsScene>
#include <vector>

#include "patch/ConnectionCable.hpp"
#include "patch/SocketWidget.hpp"
#include "widgets/SocketContainerWidget.hpp"

class ConnectionManager: public QObject {
    Q_OBJECT
private:
    QGraphicsScene* scene_ ;
    std::vector<ConnectionCable*> connections_ ;

    ConnectionCable* dragConnection_ ;
    SocketWidget* dragFromSocket_ ;
public:
    explicit ConnectionManager(QGraphicsScene* scene, QObject* parent = nullptr);

    // UI creating a connection
    void startConnection(SocketWidget* fromSocket);
    void updateDragConnection(const QPointF& scenePos);
    void finishConnection(const QPointF& scenePos);
    ConnectionRequest finishConnection(SocketWidget* toWidget);

    void cancelConnection();

    bool hasConnection(SocketWidget* socket) const ;
    
    void requestRemoveConnection(ConnectionRequest req);
    void removeSocketConnections(SocketWidget* socket);
    void removeAllConnections(SocketContainerWidget* widget);

    const std::vector<ConnectionCable*> getWidgetConnections(SocketContainerWidget* widget);

    SocketWidget* findSocketAt(const QPointF& scenePos) const ;

private:
    bool canConnect(SocketWidget* from, SocketWidget* to) const ;
    void sendConnectionApiRequest(ConnectionRequest req);

    ConnectionCable* getCable(ConnectionRequest req) const ;
    void removeCable(ConnectionCable* request);

private slots:
    void onApiDataReceived(const QJsonObject &json);
    void onWidgetPositionChanged();

signals:
    void wasModified();

};

#endif // __GUI_CONNECTION_MANAGER_HPP_