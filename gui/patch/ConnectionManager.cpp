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

ConnectionManager::ConnectionManager(QGraphicsScene* scene, QObject* parent): 
    QObject(parent), 
    scene_(scene), 
    dragConnection_(nullptr), 
    dragFromSocket_(nullptr)
{}

void ConnectionManager::startConnection(SocketWidget* fromSocket){
    if (!fromSocket) return ;
    
    fromSocket->grabMouse();
    dragFromSocket_ = fromSocket ;
    dragConnection_ = new ConnectionCable(fromSocket);
    scene_->addItem(dragConnection_);
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
        connections_.push_back(dragConnection_);


        // connect to position changes
        SocketContainerWidget* fromWidget = dragFromSocket_->getParent();
        SocketContainerWidget* toWidget = toSocket->getParent();

        // Note: other logic prohibits self connections, and we won't get 
        // this far if these are null pointers, so let's not worry about safeguards
        connect(fromWidget, &SocketContainerWidget::positionChanged, 
            this, &ConnectionManager::onWidgetPositionChanged);
        connect(toWidget, &SocketContainerWidget::positionChanged, 
            this, &ConnectionManager::onWidgetPositionChanged);

        qDebug() << "Connection created:" << dragFromSocket_->getName() 
                 << "to" << toSocket->getName() ;

        sendConnectionApiRequest(dragFromSocket_, toSocket, fromWidget, toWidget);
    } else {
        // Invalid connection
        scene_->removeItem(dragConnection_);
        delete dragConnection_ ;
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
    for ( const ConnectionCable* cable : connections_ ) {
        if (cable->getFromSocket() == socket || cable->getToSocket() == socket) {
            return true;
        }
    }
    return false;
}

void ConnectionManager::removeConnection(SocketWidget* socket){
    for (auto it = connections_.begin(); it != connections_.end();) {
        ConnectionCable* cable = *it;
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
            qDebug() << "removing cable connection:" << fromText << "->" << toText ;

            scene_->removeItem(cable);
            delete cable ;
            it = connections_.erase(it);
        } else {
            ++it;
        }
    }
}

void ConnectionManager::removeConnection(ConnectionCable* cable){
    if ( !cable ) return ;
    if ( cable->scene() ){
        cable->scene()->removeItem(cable);
    }
    connections_.erase(std::remove(connections_.begin(), connections_.end(), cable), connections_.end());
    delete cable ;
}

void ConnectionManager::removeAllConnections(SocketContainerWidget* module){
    for (SocketWidget* socket : module->getSockets()) {
        removeConnection(socket);
    }
}

const std::vector<ConnectionCable*> ConnectionManager::getWidgetConnections(SocketContainerWidget* widget){
    std::vector<ConnectionCable*> widgetCables ;
    for ( auto cable : connections_ ){
        if (cable->involvesWidget(widget)) widgetCables.push_back(cable);
    }
    return widgetCables ;
}

void ConnectionManager::sendConnectionApiRequest(
    SocketWidget* fromSock, SocketWidget* toSock, 
    SocketContainerWidget* fromWidget, SocketContainerWidget* toWidget
){
    SocketWidget* outputSock ;
    SocketWidget* inputSock ;
    SocketContainerWidget* outputWidget ;
    SocketContainerWidget* inputWidget ;

    if ( fromSock->isInput() ){
        inputSock = fromSock ;
        inputWidget = fromWidget ;
        outputSock = toSock ;
        outputWidget = toWidget ;
    } else {
        inputSock = toSock ;
        inputWidget = toWidget ;
        outputSock = fromSock ;
        outputWidget = fromWidget ;
    }

    QJsonObject obj ;
    QJsonObject input ;
    QJsonObject output ;

    output["socket"] = static_cast<int>(outputSock->getType());
    input["socket"] = static_cast<int>(inputSock->getType());
    
    // if the widgets are component widgets, then we collect the additional information
    auto outputModule = dynamic_cast<ComponentWidget*>(outputWidget);
    auto inputModule = dynamic_cast<ComponentWidget*>(inputWidget);
    if ( inputModule ){
        input["id"] = inputModule->getID();
        input["is_module"] = inputModule->getComponentDescriptor().type.isModule();
    }
    if ( outputModule ){
        output["id"] = outputModule->getID();
        output["is_module"] = outputModule->getComponentDescriptor().type.isModule();
    } 

    // if it's modulation, we need to capture the parameter to be modulated
    if ( inputSock->getType() == SocketType::ModulationInput ){
        input["parameter"] = static_cast<int>(parameterFromString(inputSock->getName().toStdString()));
    }

    obj["action"] = "create_connection" ;
    obj["output"] = output ;
    obj["input"] = input ;
    
    ApiClient::instance()->sendMessage(obj);
}


void ConnectionManager::onWidgetPositionChanged(){
    SocketContainerWidget* widget = dynamic_cast<SocketContainerWidget*>(sender());
    if (!widget) return ;
    
    for (ConnectionCable* connection: connections_ ){
        if (connection->involvesWidget(widget)){
            connection->updatePath();
        }
    }
}