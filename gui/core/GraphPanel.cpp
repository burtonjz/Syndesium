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

#include "core/GraphPanel.hpp"
#include "core/ApiClient.hpp"
#include "core/Theme.hpp"
#include "meta/ComponentDescriptor.hpp"
#include "meta/ComponentRegistry.hpp"
#include "patch/ConnectionManager.hpp"
#include "widgets/ComponentDetailWidget.hpp"
#include "widgets/SocketContainerWidget.hpp"
#include "widgets/ComponentWidget.hpp"
#include "types/ComponentType.hpp"

#include <QWheelEvent>
#include <QKeyEvent>
#include <QPainter>
#include <QDebug>
#include <QGraphicsProxyWidget>
#include <QDialog>
#include <QVBoxLayout>
#include <QLabel>
#include <QSlider>
#include <QSpinBox>
#include <QDoubleSpinBox>
#include <QPushButton>
#include <QToolTip>
#include <QMenu>

#include <QJsonObject>
#include <QJsonArray>
#include <qdebug.h>
#include <qevent.h>
#include <qgraphicsitem.h>
#include <qgraphicsscene.h>
#include <qjsonobject.h>
#include <qlogging.h>
#include <qnamespace.h>
#include <qvarlengtharray.h>

GraphPanel::GraphPanel(QWidget* parent):
    QGraphicsView(parent),
    isDraggingConnection_(false)
{
    setupScene();
    createContextMenuActions();

    addMidiInput();
    addAudioOutput();

    setFocusPolicy(Qt::StrongFocus);
    setEnabled(true);
    setMouseTracking(true);

    // connections
    connect(ApiClient::instance(), &ApiClient::dataReceived, this, &GraphPanel::onApiDataReceived);
    connect(connectionManager_, &ConnectionManager::wasModified, this, &GraphPanel::wasModified);
}

GraphPanel::~GraphPanel(){
    delete connectionManager_ ;
}

void GraphPanel::setupScene(){
    scene_ = new QGraphicsScene(this);
    scene_->setSceneRect(-2000,-2000, 4000, 4000);
    setScene(scene_);

    connectionManager_ = new ConnectionManager(scene_, this);

    // view properties
    setRenderHint(QPainter::Antialiasing);
    setDragMode(QGraphicsView::RubberBandDrag);
    setRubberBandSelectionMode(Qt::IntersectsItemBoundingRect);

    // zooming / panning
    setTransformationAnchor(QGraphicsView::AnchorUnderMouse);
    setResizeAnchor(QGraphicsView::AnchorUnderMouse);
}

void GraphPanel::createContextMenuActions(){
    disconnectAllAct_ = new QAction("Disconnect All",this) ;
    connect(disconnectAllAct_, &QAction::triggered, connectionManager_, 
        [this](){ connectionManager_->removeSocketConnections(clickedSocket_);}
    );
}

void GraphPanel::addComponent(int id, ComponentType type){
    auto* component = new ComponentWidget(id, type);
    auto* detail = new ComponentDetailWidget(id,type, this);

    widgets_.push_back(component);
    details_.push_back(detail);

    // dynamic connections
    connect(component, &SocketContainerWidget::needsZUpdate, this, &GraphPanel::onWidgetZUpdate);
    connect(component, &SocketContainerWidget::positionChanged, this, &GraphPanel::wasModified );
    connect(detail, &ComponentDetailWidget::wasModified, this, &GraphPanel::wasModified );

    scene_->addItem(component);
    for ( auto socket : component->getSockets() ){
        scene_->addItem(socket);
    }
    
    component->setPos(0,0); // TODO: dynamically place the module somewhere currently empty on the scene
    
    qDebug() << "Created component:" << component->getName() << "at position:" << component->pos() ;
    emit wasModified() ;
}

void GraphPanel::addAudioOutput(){
    audioOut_ = new SocketContainerWidget("Audio Output Device");
    audioOut_->createSockets({{SocketType::SignalInput, "Audio In"}});
    scene_->addItem(audioOut_);

    widgets_.push_back(audioOut_);
    for ( auto socket : audioOut_->getSockets() ){
        scene_->addItem(socket);
    }

    connect(audioOut_, &SocketContainerWidget::needsZUpdate, this, &GraphPanel::onWidgetZUpdate);

    qDebug() << "Created Audio Output Device Widget:" << audioOut_->getName() << "at position:" << audioOut_->pos() ;
}

void GraphPanel::addMidiInput(){
    midiIn_ = new SocketContainerWidget("MIDI Input Device");
    midiIn_->createSockets({{SocketType::MidiOutput, "MIDI Out"}});
    scene_->addItem(midiIn_);

    widgets_.push_back(midiIn_);
    for ( auto socket : midiIn_->getSockets() ){
        scene_->addItem(socket);
    }
    
    connect(midiIn_, &SocketContainerWidget::needsZUpdate, this, &GraphPanel::onWidgetZUpdate);
    qDebug() << "Created Midi Input Device Widget:" << midiIn_->getName() << "at position:" << midiIn_->pos() ;
}

SocketContainerWidget* GraphPanel::getWidget(int componentId){
    for ( auto widget : widgets_ ){
        auto component = dynamic_cast<ComponentWidget*>(widget);
        if ( component && component->getID() == componentId){
            return widget ;
        }
    }
    return nullptr ;
}

void GraphPanel::deleteSelectedModules(){
    QList<QGraphicsItem*> selectedItems = scene_->selectedItems() ;

    for ( QGraphicsItem* item: selectedItems ){
        // if it's a socket, we need to start a drag
        if ( ComponentWidget* module = qgraphicsitem_cast<ComponentWidget*>(item) ){
            connectionManager_->removeAllConnections(module);
            auto it = std::find(widgets_.begin(), widgets_.end(), module);
            if ( it != widgets_.end() ) widgets_.erase(it) ;

            scene_->removeItem(module);
            module->deleteLater();

            qDebug() << "Deleted module:" << module->getComponentDescriptor().name ;
            emit wasModified();
        }
    }
}

QJsonArray GraphPanel::getComponentPositions() const {
    QJsonArray positions ;
    for ( auto w : widgets_ ){
        auto component = dynamic_cast<ComponentWidget*>(w);
        if ( component ){
            auto pos = component->pos();
            positions.append(QJsonObject{
                {"ComponentId", component->getID()},
                {"xpos", pos.x()},
                {"ypos", pos.y()}
            });
        }
    }
    return positions ;
}

void GraphPanel::loadConnection(const QJsonObject& request){
    int inboundId = request["inbound"]["id"].toInt(-1);
    int outboundId = request["outbound"]["id"].toInt(-1);
    SocketType inboundType = static_cast<SocketType>(request["inbound"]["socketType"].toInt());
    SocketType outboundType = static_cast<SocketType>(request["outbound"]["socketType"].toInt());

    SocketContainerWidget* inboundWidget = nullptr ;
    SocketContainerWidget* outboundWidget = nullptr ;

    if ( inboundId == -1 && inboundType == SocketType::SignalInput ){
        inboundWidget = audioOut_ ;
    } else {
        inboundWidget = getWidget(inboundId);
    }

    if ( outboundId == -1 && outboundType == SocketType::MidiOutput){
        outboundWidget = midiIn_ ;
    } else {
        outboundWidget = getWidget(outboundId);
    }

    if ( ! inboundWidget || ! outboundWidget ){
        qWarning() << "Json connection not successfully loaded: invalid connection request" ;
        return ;
    }

    // find specific sockets now
    SocketWidget* inboundSocket ;
    SocketWidget* outboundSocket ;

    if ( inboundType == SocketType::ModulationInput ){
        ParameterType modulatedParam = static_cast<ParameterType>(request["inbound"]["parameter"].toInt());
        inboundSocket = getWidgetSocket(inboundWidget, inboundType, modulatedParam);
        outboundSocket = getWidgetSocket(outboundWidget, outboundType);
    } else {
        inboundSocket = getWidgetSocket(inboundWidget, inboundType);
        outboundSocket = getWidgetSocket(outboundWidget, outboundType);
    }
    
    if ( ! inboundSocket || ! outboundSocket ){
        qWarning() << "Json connection not successfully loaded: sockets not found" ;
    }
    
    connectionManager_->startConnection(outboundSocket);
    connectionManager_->finishConnection(inboundSocket);
}

void GraphPanel::loadPositions(const QJsonObject& request){
    // Loop through each position object
    auto positions = request["positions"].toArray();

    for (const QJsonValue& value : positions) {
        QJsonObject posObj = value.toObject();
        
        int componentID = posObj["ComponentId"].toInt();
        int xpos = posObj["xpos"].toInt();
        int ypos = posObj["ypos"].toInt();
        
        auto c = getWidget(componentID);
        if ( c ){
            c->setPos(xpos,ypos);
        }
    }
}


SocketWidget* GraphPanel::getWidgetSocket(SocketContainerWidget* w, SocketType t, ParameterType p){
    if (!w){
        qWarning() << "invalid widget specified." ;
        return nullptr ;
    }

    for ( auto s : w->getSockets() ){
        if ( s->getType() == t ){
            if ( t == SocketType::ModulationInput ){
                if ( p == parameterFromString(s->getName().toStdString())){
                    return s ;
                }
            } else {
                return s ;
            }
        }
    }

    return nullptr ;
}

void GraphPanel::keyPressEvent(QKeyEvent* event){
    switch (event->key()){
        case Qt::Key_Delete:
        case Qt::Key_Backspace:
            deleteSelectedModules();
            break ;
        case Qt::Key_Escape:
            connectionManager_->cancelConnection();
            break ;
        default:
            QGraphicsView::keyPressEvent(event);
    }
}

void GraphPanel::mouseMoveEvent(QMouseEvent* event){
    QPointF scenePos = mapToScene(event->pos());
    SocketWidget* w = connectionManager_->findSocketAt(scenePos);

    // resolve hover events for socket widgets
    if ( lastHovered_ ){
        lastHovered_->setHovered(false);
        lastHovered_ = nullptr ;
    }

    if ( w ){
        w->setHovered(true);
        lastHovered_ = w ;
    }
    
    // handle socket connection dragging
    if ( isDraggingConnection_ ){
        connectionManager_->updateDragConnection(scenePos);

        // manually show tool tip if hovering
        if (w && !w->toolTip().isEmpty()){
            QToolTip::showText(QCursor::pos(), w->toolTip());
            return ;
        }

        // hide tool tip if no longer found
        QToolTip::hideText();

        event->accept();
        return ;
    }

    QGraphicsView::mouseMoveEvent(event);
}

void GraphPanel::mousePressEvent(QMouseEvent* event){
    QPointF scenePos = mapToScene(event->pos());

    if ( event->button() == Qt::LeftButton ){
        if ( SocketWidget* w = connectionManager_->findSocketAt(scenePos) ){
            isDraggingConnection_ = true ;
            connectionManager_->startConnection(w);
            event->accept();
            return ;
        }
    }

    if ( event->button() == Qt::RightButton ){
        // TODO: maybe if we right click a module we can do some stuff...
    }

    QGraphicsView::mousePressEvent(event); // pass event through
}

void GraphPanel::mouseDoubleClickEvent(QMouseEvent* event){
    QPointF scenePos = mapToScene(event->pos());

    // Launch ComponentWidget
    QGraphicsItem* item = scene()->itemAt(scenePos, transform());
    while (item){
        if ( ComponentWidget* w = dynamic_cast<ComponentWidget*>(item)){ 
            componentDoubleClicked(w);
        } 
        item = item->parentItem();
    }
}

void GraphPanel::mouseReleaseEvent(QMouseEvent* event){
    QPointF scenePos = mapToScene(event->pos());

    if (event->button() == Qt::LeftButton && isDraggingConnection_ ) {
        isDraggingConnection_ = false;
        connectionManager_->finishConnection(scenePos);
        event->accept();
        return ;
    }

    QGraphicsView::mouseReleaseEvent(event);
}

void GraphPanel::contextMenuEvent(QContextMenuEvent *event){
    QPointF scenePos = mapToScene(event->pos());

    // right clicking on a socket
    if ( SocketWidget* w = connectionManager_->findSocketAt(scenePos) ){
        clickedSocket_ = w ;
        QMenu menu(this);\
        menu.addAction(disconnectAllAct_);
        menu.exec(event->globalPos());
        return ;
    }
    clickedSocket_ = nullptr ;

}

void GraphPanel::wheelEvent(QWheelEvent* event){
    if ( event->angleDelta().y() > 0 ){
        scale(WHEEL_SCALE_FACTOR, WHEEL_SCALE_FACTOR);
    } else {
        scale( 1.0 / WHEEL_SCALE_FACTOR, 1.0 / WHEEL_SCALE_FACTOR );
    }
}

void GraphPanel::drawBackground(QPainter* painter, const QRectF& rect){
    // Draw Grid
    painter->setPen(QPen(Theme::GRAPH_GRID_COLOR, 1));

    qreal left = int(rect.left()) - (int(rect.left()) % int(GRID_SIZE));
    qreal top = int(rect.top()) - (int(rect.top()) % int(GRID_SIZE));

    QVarLengthArray<QLineF, 100> lines ;

    for (qreal x = left; x < rect.right(); x+= GRID_SIZE){
        lines.append(QLineF(x, rect.top(), x, rect.bottom()));
    }

    for (qreal y = top; y < rect.bottom(); y += GRID_SIZE){
        lines.append(QLineF(rect.left(), y, rect.right(), y));
    }

    painter->drawLines(lines.data(), lines.size());

}

void GraphPanel::onComponentAdded(ComponentType type){
    QJsonObject obj ;
    auto descriptor = ComponentRegistry::getComponentDescriptor(type);

    obj["action"] = "add_component" ;
    obj["name"] = QString::fromStdString(descriptor.name) ;
    obj["type"] = static_cast<int>(type);
    ApiClient::instance()->sendMessage(obj); 
}

void GraphPanel::onApiDataReceived(const QJsonObject& json){
    QString action = json["action"].toString();

    if ( action == "add_component" ){
        if ( json["status"] != "success" ){
            qDebug() << "module was not successfully added." ;
            return ;
        }

        int id = json["componentId"].toInt();
        ComponentType type = static_cast<ComponentType>(json["type"].toInt());
        addComponent(id, type);
    }

    // if the load api creates a connection, we need to draw it here
    if ( action == "create_connection" ){
        if ( json["status"] == "success" && ! json.contains("connectionId") ){
            loadConnection(json);
        }
    }

    if ( action == "load_configuration" ){
        if ( json["status"] == "success"){
            loadPositions(json);
        }
    }
}

void GraphPanel::componentDoubleClicked(ComponentWidget* widget){
    int id = widget->getID();
    ComponentType type = widget->getComponentDescriptor().type ;

    for ( auto d : details_ ){
        if( d->getID() == id ){
            if ( d->isVisible() ){
                d->hide();
            } else {
                d->show();
                d->raise();
            }
            return ;
        }
    }

    qDebug() << "No detail component found.";
}

void GraphPanel::onWidgetZUpdate(){
    SocketContainerWidget* widget = dynamic_cast<SocketContainerWidget*>(sender());
    int maxZ = 0 ;
    for ( auto w : widgets_ ){
        if ( w != widget && w->zValue() > maxZ ){
            maxZ = w->zValue();
        }
    }

    if ( maxZ != 0 && widget->zValue() == maxZ ) return ;

    widget->setZValue( maxZ + 1 );

    // cables go ahead of sockets, behind widgets
    auto widgetCables = connectionManager_->getWidgetConnections(widget);
    for ( auto* cable : widgetCables ){
        cable->setZValue( maxZ + 0.9 ); 
    }

    // lastly, sockets (this doesn't work because sockets are parented :( )
    auto sockets = widget->getSockets();
    for ( auto* socket: sockets){
        socket->setZValue(maxZ + 0.8);
    }
 
}