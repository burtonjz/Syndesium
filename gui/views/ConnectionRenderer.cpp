/*
 * Copyright (C) 2026 Jared Burton
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

#include "views/ConnectionRenderer.hpp"

#include "widgets/SocketContainerWidget.hpp"

ConnectionRenderer::ConnectionRenderer(
    QGraphicsScene* scene,
    ConnectionManager* manager,
    ISocketLookup* socketLookup,
    QObject* parent
):
    QObject(parent),
    scene_(scene),
    manager_(manager),
    socketLookup_(socketLookup),
    dragCable_(nullptr),
    dragFromSocket_(nullptr)
{
    connect(
        manager_, 
        &ConnectionManager::connectionAdded, 
        this, 
        &ConnectionRenderer::onConnectionAdded
    );

    connect(
        manager_, 
        &ConnectionManager::connectionRemoved, 
        this, 
        &ConnectionRenderer::onConnectionRemoved
    );
}

// creating new cables
void ConnectionRenderer::startDrag(SocketWidget* fromSocket){
    if (!fromSocket || isDragging() ) return ;

    fromSocket->grabMouse();
    dragFromSocket_ = fromSocket ;
    dragCable_ = new ConnectionCable(fromSocket);
    scene_->addItem(dragCable_);
    dragCable_->setZValue(1e6); // on top of everything
}

void ConnectionRenderer::updateDrag(const QPointF& scenePos){
    if (dragCable_) dragCable_->setEndpoint(scenePos) ;
}

void ConnectionRenderer::finishDrag(const QPointF& scenePos){
    ConnectionRequest request ;
    
    if ( !dragCable_ || !dragFromSocket_ ){
        qWarning() << "drag connection not available. Unable to finish drag.";
        return ;
    }

    dragFromSocket_->ungrabMouse();

    SocketWidget* toSocket = socketLookup_->findSocketAt(scenePos);
    if ( !toSocket ){
        qInfo() << "No socket endpoint specified for drag cable. Cancelling connection." ;
        cancelDrag();
        return ;
    }
    dragCable_->setToSocket(toSocket);

    if ( !dragCable_->isCompatible(toSocket)){
        cancelDrag();
        return ;
    }

    request = dragCable_->toConnectionRequest();

    manager_->requestConnectionEvent(request);

    // cleanup temporary cable
    cancelDrag();

}

void ConnectionRenderer::cancelDrag(){
    if ( dragCable_ ){
        scene_->removeItem(dragCable_);
        delete dragCable_ ;
        dragCable_ = nullptr ;
    }
    dragFromSocket_ = nullptr ;
}

bool ConnectionRenderer::isDragging() const {
    return dragCable_ != nullptr ;
}

void ConnectionRenderer::removeSocketConnections(SocketWidget* s){
    for ( const auto& c : cables_ ){
        if ( c->getFromSocket() == s || c->getToSocket() == s ){
            auto req = c->toConnectionRequest();
            req.remove = true ;
            manager_->requestConnectionEvent(req);
        }
    }
}

const std::vector<ConnectionCable*> ConnectionRenderer::getWidgetConnections(SocketContainerWidget* widget) const {
    std::vector<ConnectionCable*> c ;
    for (const auto& cable : cables_ ) {
        if (cable->involvesWidget(widget)) c.push_back(cable);
    }
    return c ;
}

void ConnectionRenderer::onWidgetPositionChanged(){
    SocketContainerWidget* widget = dynamic_cast<SocketContainerWidget*>(sender());
    if (!widget) return ;
    
    for ( const auto& cable : cables_ ) {
        if (cable->involvesWidget(widget)){
            cable->updatePath();
        }
    }
}

void ConnectionRenderer::onConnectionAdded(const ConnectionRequest& req){
    SocketWidget* outbound = socketLookup_->findSocket(
        req.outboundSocket, 
        req.outboundID, 
        req.outboundIdx
    );
    SocketWidget*  inbound = socketLookup_->findSocket(
        req.inboundSocket, 
        req.inboundID, 
        req.inboundIdx, 
        req.inboundParameter
    );

    if ( !outbound || !inbound ){
        qWarning() << "did not find sockets to draw connection cable. Please investigate" ;
        return ;
    }

    ConnectionCable* c = new ConnectionCable(outbound, inbound);
    cables_.push_back(c);
    scene_->addItem(c);
    c->setZValue(std::max(inbound->zValue(), outbound->zValue()));
}

void ConnectionRenderer::onConnectionRemoved(const ConnectionRequest& req){
    for ( auto c : cables_ ){
        if ( c->toConnectionRequest() == req ){
            scene_->removeItem(c);
            delete c ;
            cables_.erase(std::remove(cables_.begin(), cables_.end(), c), cables_.end());
        }
    }
}
