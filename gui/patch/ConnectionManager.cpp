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
#include "types/ParameterType.hpp"
#include "types/SocketType.hpp"
#include "widgets/ComponentWidget.hpp"
#include "widgets/SocketContainerWidget.hpp"
#include "core/ApiClient.hpp"

#include <QGraphicsItem>
#include <QDebug>
#include <qdebug.h>
#include <qgraphicsitem.h>
#include <algorithm>

ConnectionManager::ConnectionManager(QGraphicsScene* scene, QObject* parent): 
    QObject(parent), 
    scene_(scene), 
    currentConnectionID_(0),
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
    if (!dragConnection_ || !dragFromSocket_) {
        qDebug() << "Cancelling connection." ;
        cancelConnection();
        return ;
    }
    
    dragFromSocket_->ungrabMouse();

    SocketWidget* toSocket = findSocketAt(scenePos);
    if (toSocket && canConnect(dragFromSocket_, toSocket)) {
        // Complete the connection
        dragConnection_->setToSocket(toSocket);
    
        // connect to position changes
        SocketContainerWidget* fromWidget = dragFromSocket_->getParent();
        SocketContainerWidget* toWidget = toSocket->getParent();

        // set connection cable layering
        dragConnection_->setZValue(std::max(fromWidget->zValue(), toWidget->zValue()) - 0.1);

        connect(fromWidget, &SocketContainerWidget::positionChanged, 
            this, &ConnectionManager::onWidgetPositionChanged);
        connect(toWidget, &SocketContainerWidget::positionChanged, 
            this, &ConnectionManager::onWidgetPositionChanged);

        // add connection to map
        ConnectionID id = currentConnectionID_++ ;
        connections_[id] = dragConnection_ ;

        sendConnectionApiRequest(id);
    } else {
        cancelConnection();
    }
    
    dragConnection_ = nullptr;
    dragFromSocket_ = nullptr;
}

void ConnectionManager::cancelConnection()
{
    if (dragConnection_) {
        scene_->removeItem(dragConnection_);
        delete dragConnection_ ;
        dragConnection_ = nullptr;
    }
    dragFromSocket_ = nullptr;
}

void ConnectionManager::cancelConnection(ConnectionID connection){
    auto cable = connections_[connection];
    if (cable){
        scene_->removeItem(cable);
        delete cable ;
        connections_.erase(connection);
    }
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
    for (auto it = connections_.begin(); it != connections_.end();) {
        if ( it->second->getFromSocket() == socket || it->second->getToSocket() == socket ) {
            return true;
        }
    }
    return false;
}

void ConnectionManager::removeConnection(ConnectionID connection){
    ConnectionCable* cable = connections_[connection];
    if ( !cable ) return ;
    if ( cable->scene() ){
        cable->scene()->removeItem(cable);
    }

    sendConnectionApiRequest(connection, true);
}

void ConnectionManager::removeSocketConnections(SocketWidget* socket){
    for (const auto& [id, cable] : connections_) {
        if (cable->getFromSocket() == socket || cable->getToSocket() == socket) {
            QString fromText = cable->getFromSocket()
                ? QString("%1%2")
                    .arg(cable->getFromSocket()->getParent()->getName())
                    .arg(cable->getFromSocket()->getName())
                : "null" ;

            QString toText = cable->getToSocket()
                ? QString("%1%2")
                    .arg(cable->getToSocket()->getParent()->getName())
                    .arg(cable->getToSocket()->getName())
                : "null" ;

            sendConnectionApiRequest(id, true);
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
    for (const auto& [id, cable] : connections_) {
        if (cable->involvesWidget(widget)) widgetCables.push_back(cable);
    }
    return widgetCables ;
}

void ConnectionManager::sendConnectionApiRequest(ConnectionID connection, bool remove){
    QJsonObject obj ;
    QJsonObject input ;
    QJsonObject output ;

    if ( remove ){
        obj["action"] = "remove_connection" ;
    } else {
        obj["action"] = "create_connection" ;
    }
    
    ConnectionCable* cable = connections_[connection];

    if (!cable){
        qDebug() << "invalid connection id provided" ;
        return ;
    }

    auto outboundSocket = cable->getOutboundSocket();
    auto inboundSocket = cable->getInboundSocket();

    auto outboundComponent = dynamic_cast<ComponentWidget*>(outboundSocket->getParent());
    auto inboundComponent = dynamic_cast<ComponentWidget*>(inboundSocket->getParent());

    output["socket"] = static_cast<int>(outboundSocket->getType());
    input["socket"] = static_cast<int>(inboundSocket->getType());
    
    // if the widgets are component widgets, then we collect the additional information
    if ( inboundComponent ) input["id"] = inboundComponent->getID();
    if ( outboundComponent ) output["id"] = outboundComponent->getID();

    // if it's modulation, we need to capture the parameter to be modulated
    if ( inboundSocket->getType() == SocketType::ModulationInput ){
        input["parameter"] = static_cast<int>(parameterFromString(inboundSocket->getName().toStdString()));
    }
    
    obj["connectionID"] = connection ;
    obj["outbound"] = output ;
    obj["inbound"] = input ;
    
    ApiClient::instance()->sendMessage(obj);

}

void ConnectionManager::onApiDataReceived(const QJsonObject &json){
    QString action = json["action"].toString();
    bool success = json["status"].toString() == "success" ;

    if ( action == "create_connection" && !success ){
        ConnectionID id = json["connectionID"].toInt(-1);
        removeConnection(id);
        return ;
    } 

    if ( action == "remove_connection" && success ){
        ConnectionID id = json["connectionID"].toInt(-1);
        ConnectionCable* cable = connections_[id];
        if ( !cable ){
            qDebug() << "connectionID value " << id << "is not present in map. Cannot remove connection." ;
            return ;
        }

        QString fromText = cable->getFromSocket()
            ? QString("%1 %2")
                .arg(cable->getFromSocket()->getParent()->getName())
                .arg(cable->getFromSocket()->getName())
            : "null" ;

        QString toText = cable->getToSocket()
            ? QString("%1 %2")
                .arg(cable->getToSocket()->getParent()->getName())
                .arg(cable->getToSocket()->getName())
            : "null" ;
        qDebug() << "removing cable connection: " << fromText << " -> " << toText ;
        cancelConnection(id);
    }
}

void ConnectionManager::onWidgetPositionChanged(){
    SocketContainerWidget* widget = dynamic_cast<SocketContainerWidget*>(sender());
    if (!widget) return ;
    
    for (const auto& [id, cable] : connections_) {
        if (cable->involvesWidget(widget)){
            cable->updatePath();
        }
    }
}