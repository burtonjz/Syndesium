#include "patch/ConnectionManager.hpp"
#include "patch/ConnectionCable.hpp"
#include "widgets/ModuleWidget.hpp"
#include "widgets/SocketContainerWidget.hpp"
#include "core/ApiClient.hpp"

#include <QGraphicsItem>
#include <QDebug>
#include <qdebug.h>

ConnectionManager::ConnectionManager(QGraphicsScene* scene, QObject* parent): 
    QObject(parent), 
    scene_(scene), 
    dragConnection_(nullptr), 
    dragFromSocket_(nullptr)
{
}

void ConnectionManager::startConnection(SocketWidget* fromSocket){
    if (!fromSocket) return ;
    
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
    
    SocketWidget* toSocket = findSocketAt(scenePos);
    if (toSocket && canConnect(dragFromSocket_, toSocket)) {
        // Complete the connection
        dragConnection_->setToSocket(toSocket);
        connections_.append(dragConnection_);

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
    connections_.removeAll(cable);
    delete cable ;
}

void ConnectionManager::removeAllConnections(SocketContainerWidget* module){
    for (SocketWidget* socket : module->getSockets()) {
        removeConnection(socket);
    }
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
    
    // test to see if widget is module
    auto outputModule = dynamic_cast<ModuleWidget*>(outputWidget);
    auto inputModule = dynamic_cast<ModuleWidget*>(inputWidget);
    if ( inputModule ) input["id"] = inputModule->getID();
    if ( outputModule ) output["id"] = outputModule->getID();

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