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

#include "interfaces/ISocketLookup.hpp"
#include "requests/ConnectionRequest.hpp"

#include <QObject>
#include <QGraphicsScene>
#include <vector>

class ConnectionManager: public QObject {
    Q_OBJECT
private:
    std::vector<ConnectionRequest> connections_ ;
    ISocketLookup* socketLookup_ ;

public:
    explicit ConnectionManager(QObject* parent = nullptr);

    // backend confirmed successful connection
    void loadConnection(const ConnectionRequest& req);
    bool hasConnection(SocketWidget* socket) const ;
     
    void requestConnectionEvent(const ConnectionRequest& req); // add or remove

private:
    void sendConnectionApiRequest(ConnectionRequest req);

    bool connectionExists(ConnectionRequest req) const ;

private slots:
    void onApiDataReceived(const QJsonObject &json);

signals:
    void connectionAdded(const ConnectionRequest& req);
    void connectionRemoved(const ConnectionRequest& req);

};

#endif // __GUI_CONNECTION_MANAGER_HPP_