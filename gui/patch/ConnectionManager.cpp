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

#include "patch/ConnectionManager.hpp"
#include "patch/ConnectionCable.hpp"
#include "types/ConnectionRequest.hpp"
#include "util/util.hpp"
#include "widgets/SocketContainerWidget.hpp"
#include "core/ApiClient.hpp"

#include <QGraphicsItem>
#include <QDebug>
#include <qdebug.h>
#include <qgraphicsitem.h>
#include <algorithm>
#include <qjsonobject.h>

ConnectionManager::ConnectionManager(QGraphicsScene* scene, QObject* parent): 
    QObject(parent), 
    scene_(scene), 
    dragConnection_(nullptr), 
    dragFromSocket_(nullptr)
{
    connect(ApiClient::instance(), &ApiClient::dataReceived, this, &ConnectionManager::onApiDataReceived);
}

void ConnectionManager::startConnection(SocketWidget* fromSocket){
    if (!fromSocket) return ;
    
    fromSocket->grabMouse();
    dragFromSocket_ = fromSocket ;
    dragConnection_ = new ConnectionCable(fromSocket);
    scene_->addItem(dragConnection_);
    dragConnection_->setZValue(1e6);
}

void ConnectionManager::updateDragConnection(const QPointF& scenePos){
    if (dragConnection_) dragConnection_->setEndpoint(scenePos) ;
}

void ConnectionManager::finishConnection(const QPointF& scenePos){
    SocketWidget* toSocket = findSocketAt(scenePos);

    auto request = finishConnection(toSocket);
    sendConnectionApiRequest(request);    
}

ConnectionRequest ConnectionManager::finishConnection(SocketWidget* toSocket){
    if (!dragConnection_ || !dragFromSocket_) {
        qWarning() << "from connection not specified. This shouldn't happen!" ;
        ConnectionRequest req ;
        req.inboundID = -1 ;
        req.outboundID = -1 ;
        return req ;
    }

    dragFromSocket_->ungrabMouse();
    
    if (!toSocket){
        qWarning() << "invalid toSocket specified." ;
        cancelConnection();
        ConnectionRequest req ;
        req.inboundID = -1 ;
        req.outboundID = -1 ;
        return req ;
    }

    if ( ! canConnect(dragFromSocket_, toSocket)){
        qWarning() << "invalid socket combination. Sockets cannot be connected" ;
        cancelConnection();
        ConnectionRequest req ;
        req.inboundID = -1 ;
        req.outboundID = -1 ;
        return req ;
    }

    // Complete the connection
    dragConnection_->setToSocket(toSocket);

    // connect to position changes
    SocketContainerWidget* fromWidget = dragFromSocket_->getParent();
    SocketContainerWidget* toWidget = toSocket->getParent();

    connect(fromWidget, &SocketContainerWidget::positionChanged, 
        this, &ConnectionManager::onWidgetPositionChanged);
    connect(toWidget, &SocketContainerWidget::positionChanged, 
        this, &ConnectionManager::onWidgetPositionChanged);

    // set connection cable layering
    dragConnection_->setZValue(std::max(fromWidget->zValue(), toWidget->zValue()) - 0.1);

    // add connection to map
    connections_.push_back(dragConnection_);
    auto request = dragConnection_->toConnectionRequest();

    dragConnection_ = nullptr ;
    dragFromSocket_ = nullptr ;

    return request ;
}

void ConnectionManager::cancelConnection(){
    if (dragConnection_) {
        scene_->removeItem(dragConnection_);
        delete dragConnection_ ;
        dragConnection_ = nullptr;
    }
    dragFromSocket_ = nullptr;
}

SocketWidget* ConnectionManager::findSocketAt(const QPointF& scenePos) const {
    QList<QGraphicsItem*> items = scene_->items(scenePos);
    for (QGraphicsItem* item : items) {
        if (SocketWidget* socket = dynamic_cast<SocketWidget*>(item)) {
            return socket ;
        }
    }
    return nullptr ;
}

bool ConnectionManager::canConnect(SocketWidget* from, SocketWidget* to) const {
    if (!from || !to) return false;
    
    // Use the cable's compatibility check
    ConnectionCable tempCable(from);
    return tempCable.isCompatible(to);
}

bool ConnectionManager::hasConnection(SocketWidget* socket) const {
    for ( auto c : connections_ ) {
        if ( c->getFromSocket() == socket || c->getToSocket() == socket ) {
            return true;
        }
    }
    return false;
}

void ConnectionManager::requestRemoveConnection(ConnectionRequest req){
    ConnectionCable* cable = getCable(req);
    if ( !cable ){
        qWarning() << "Unable to remove cable, not found." ;
        return ;
    }
    req.remove = true ;
    sendConnectionApiRequest(req);
}

void ConnectionManager::removeSocketConnections(SocketWidget* socket){
    for (const auto& cable : connections_) {
        if (cable->getFromSocket() == socket || cable->getToSocket() == socket) {
            auto req = cable->toConnectionRequest();
            req.remove = true ;
            sendConnectionApiRequest(req);
        }
    }
}

void ConnectionManager::removeAllConnections(SocketContainerWidget* module){
    for (SocketWidget* socket : module->getSockets()) {
        removeSocketConnections(socket);
    }
}

const std::vector<ConnectionCable*> ConnectionManager::getWidgetConnections(SocketContainerWidget* widget){
    std::vector<ConnectionCable*> widgetCables ;
    for (const auto& cable : connections_) {
        if (cable->involvesWidget(widget)) widgetCables.push_back(cable);
    }
    return widgetCables ;
}

void ConnectionManager::sendConnectionApiRequest(ConnectionRequest req){
    auto obj = Util::nlohmannToQJsonObject(req);
    ApiClient::instance()->sendMessage(obj);
}

ConnectionCable* ConnectionManager::getCable(ConnectionRequest request) const {
    auto it = std::find_if(connections_.begin(), connections_.end(), [&request](const ConnectionCable* cable){
        return *cable == request ;
    });

    if ( it == connections_.end() ) return nullptr ;

    return *it ;
}

void ConnectionManager::removeCable(ConnectionCable* cable){
    if (cable){
        if ( cable->scene() ){
            cable->scene()->removeItem(cable);
        }
        scene_->removeItem(cable);
        connections_.erase(std::remove(connections_.begin(), connections_.end(), cable), connections_.end());
        delete cable ;
        emit wasModified();
    }
}

void ConnectionManager::onApiDataReceived(const QJsonObject &json){
    QString action = json["action"].toString();
    bool success = json["status"].toString() == "success" ;

    if ( action == "create_connection" ){
        if ( success ){
            emit wasModified();
        } else {
            removeCable(getCable(Util::QJsonObjectToNlohmann(json)));
            return ;
        }
    } 

    if ( action == "remove_connection" && success ){
        ConnectionCable* cable = getCable(Util::QJsonObjectToNlohmann(json));

        if ( !cable ){
            qWarning() << "no cable found from json request " << json ;
            return ;
        }

        qInfo() << "removing cable connection: " << cable->toText() ;
        
        removeCable(cable);
    }
}

void ConnectionManager::onWidgetPositionChanged(){
    SocketContainerWidget* widget = dynamic_cast<SocketContainerWidget*>(sender());
    if (!widget) return ;
    
    for (const auto& cable : connections_) {
        if (cable->involvesWidget(widget)){
            cable->updatePath();
        }
    }

    emit wasModified() ;
}