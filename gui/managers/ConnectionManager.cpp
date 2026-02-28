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

#include "managers/ConnectionManager.hpp"
#include "requests/ConnectionRequest.hpp"
#include "util/util.hpp"
#include "api/ApiClient.hpp"

#include <QGraphicsItem>
#include <QDebug>
#include <qdebug.h>
#include <qgraphicsitem.h>
#include <algorithm>
#include <qjsonobject.h>

ConnectionManager::ConnectionManager(QObject* parent): 
    QObject(parent)
{
    connect(ApiClient::instance(), &ApiClient::dataReceived, this, &ConnectionManager::onApiDataReceived);
}

void ConnectionManager::loadConnection(const ConnectionRequest& req){
    if ( !req.valid() ){
        qWarning() << "Connection failed: Json is not a valid ConnectionRequest." ;
        return ;
    }

    if ( connectionExists(req) ){
        qWarning() << "Connection already exists. Will not load connection again.";
        return ;
    }
    
    // make sure sockets are real before adding the connection
    SocketWidget* inboundSocket = socketLookup_->findSocket(
        req.inboundSocket, req.inboundID, req.inboundIdx, req.inboundParameter
    );
    SocketWidget* outboundSocket = socketLookup_->findSocket(
        req.outboundSocket, req.outboundID, req.outboundIdx
    );

    if ( ! inboundSocket || ! outboundSocket ){
        qWarning() << "Json connection not successfully loaded: sockets not found";
        return ;
    }

    emit connectionAdded(req);
}

void ConnectionManager::requestConnectionEvent(const ConnectionRequest& req){
    if ( !req.valid() ){
        qDebug() << nlohmann::json(req).dump() ;
        qWarning() << "Invalid connection request created. Cancelling connection.";
        return ;
    }

    sendConnectionApiRequest(req);    
}

void ConnectionManager::sendConnectionApiRequest(ConnectionRequest req){
    auto obj = Util::nlohmannToQJsonObject(req);
    ApiClient::instance()->sendMessage(obj);
}

bool ConnectionManager::connectionExists(ConnectionRequest request) const {
    auto it = std::find_if(connections_.begin(), connections_.end(), [&request](const ConnectionRequest& r){
        return r == request ;
    });

    return it != connections_.end() ;
}

void ConnectionManager::onApiDataReceived(const QJsonObject &json){
    QString action = json["action"].toString();
    bool success = json["status"].toString() == "success" ;

    if ( action == "create_connection" && success ){
        ConnectionRequest req = Util::QJsonObjectToNlohmann(json);
        if ( connectionExists(req) ){
            qWarning() << "requested connection already exists in connection manager. Will not add again.";
            return ;
        }
        connections_.push_back(req);
        emit connectionAdded(req);
    } 

    if ( action == "remove_connection" && success ){
        ConnectionRequest req = Util::QJsonObjectToNlohmann(json);
        if ( !connectionExists(req) ){
            qWarning() << "requested connection is not present in connection manager. will not trigger removal.";
            return ;
        }
        connections_.erase(std::remove(connections_.begin(), connections_.end(), req), connections_.end());
        emit connectionRemoved(req);
    }
}