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

#include "views/GraphPanel.hpp"
#include "api/ApiClient.hpp"
#include "app/Theme.hpp"
#include "managers/ConnectionManager.hpp"
#include "util/util.hpp"
#include "graphics/GraphNode.hpp"
#include "widgets/SocketWidget.hpp"
#include "graphics/ComponentNode.hpp"
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

    connectionManager_ = new ConnectionManager(this);
    connectionRenderer_ = new ConnectionRenderer(scene_, connectionManager_, this, this);
    componentManager_ = new ComponentManager(this);

    createContextMenuActions();

    addMidiInput();
    addAudioOutput();

    setFocusPolicy(Qt::StrongFocus);
    setEnabled(true);
    setMouseTracking(true);

    // connections
    connect(
        ApiClient::instance(), &ApiClient::dataReceived, 
        this, &GraphPanel::onApiDataReceived
    );
    connect(
        componentManager_, &ComponentManager::componentAdded,
        this, &GraphPanel::onComponentAdded
    );
    connect(
        componentManager_, &ComponentManager::componentRemoved,
        this, &GraphPanel::onComponentRemoved
    );
}

GraphPanel::~GraphPanel(){

}

void GraphPanel::setupScene(){
    scene_ = new QGraphicsScene(this);
    scene_->setSceneRect(-2000,-2000, 4000, 4000);
    setScene(scene_);

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
        [this](){ connectionRenderer_->removeSocketConnections(clickedSocket_);}
    );
}

void GraphPanel::addAudioOutput(){
    audioOut_ = new GraphNode("Audio Output Device");
    audioOut_->createSockets({{SocketType::SignalInbound, "Audio In"}});
    audioOut_->setData(Qt::UserRole, 0); // output index
    scene_->addItem(audioOut_);

    nodes_.push_back(audioOut_);
    for ( auto socket : audioOut_->getSockets() ){
        scene_->addItem(socket);
    }

    connect(audioOut_, &GraphNode::needsZUpdate, this, &GraphPanel::onNodeZUpdate);
    connect(audioOut_, &GraphNode::positionChanged, connectionRenderer_, &::ConnectionRenderer::onNodePositionChanged);

    qDebug() << "Created Audio Output Device Widget:" << audioOut_->getName() << "at position:" << audioOut_->pos() ;
}

void GraphPanel::addMidiInput(){
    midiIn_ = new GraphNode("MIDI Input Device");
    midiIn_->createSockets({{SocketType::MidiOutbound, "MIDI Out"}});
    scene_->addItem(midiIn_);

    nodes_.push_back(midiIn_);
    for ( auto socket : midiIn_->getSockets() ){
        scene_->addItem(socket);
    }
    
    connect(midiIn_, &GraphNode::needsZUpdate, this, &GraphPanel::onNodeZUpdate);
    connect(midiIn_, &GraphNode::positionChanged, connectionRenderer_, &ConnectionRenderer::onNodePositionChanged);

    qDebug() << "Created Midi Input Device Widget:" << midiIn_->getName() << "at position:" << midiIn_->pos() ;
}

ComponentNode* GraphPanel::getNode(int componentId) const {
    for ( auto n : nodes_ ){
        auto cNode = dynamic_cast<ComponentNode*>(n);
        if ( cNode ){
            if ( cNode->getModel()->getId() == componentId ){
                return cNode ;
            }
        }
    }
    return nullptr ;
}

QJsonArray GraphPanel::getComponentPositions() const {
    QJsonArray positions ;
    for ( auto n : nodes_ ){
        auto component = dynamic_cast<ComponentNode*>(n);
        if ( component ){
            auto pos = component->pos();
            positions.append(QJsonObject{
                {"ComponentId", component->getModel()->getId()},
                {"xpos", pos.x()},
                {"ypos", pos.y()}
            });
        }
    }
    return positions ;
}

void GraphPanel::loadConnection(const QJsonObject& request){
    ConnectionRequest conn = Util::QJsonObjectToNlohmann(request) ;
}

void GraphPanel::loadPositions(const QJsonObject& request){
    // Loop through each position object
    auto positions = request["positions"].toArray();

    for (const QJsonValue& value : positions) {
        QJsonObject posObj = value.toObject();
        
        int componentID = posObj["ComponentId"].toInt();
        int xpos = posObj["xpos"].toInt();
        int ypos = posObj["ypos"].toInt();
        
        auto n = getNode(componentID);
        if ( n ){
            n->setPos(xpos,ypos);
        }
    }
}


SocketWidget* GraphPanel::getNodeSocket(
        GraphNode* n, SocketType t, 
        std::variant<std::monostate, size_t, ParameterType> selector 
){
    if ( !n ){
        qWarning() << "invalid widget specified." ;
        return nullptr ;
    }

    for ( auto s : n->getSockets() ){
        if ( s->getType() == t ){
            if ( t == SocketType::ModulationInbound ){
                ParameterType p = std::get<ParameterType>(selector);
                if ( p == parameterFromString(s->getName().toStdString())){
                    return s ;
                }
            } else if ( t == SocketType:: SignalInbound || t == SocketType::SignalOutbound ) {
                size_t index = std::get<size_t>(selector);
                if ( index == s->data(Qt::UserRole).value<size_t>() ){
                    return s ;
                }
            } else {
                return s ;
            }
        }
    }

    return nullptr ;
}

std::vector<ComponentNode*> GraphPanel::getSelectedComponents() const {
    auto selectedItems = scene_->selectedItems() ;
    std::vector<ComponentNode*> nodes ;

    for ( QGraphicsItem* item: selectedItems ){
        ComponentNode* node = dynamic_cast<ComponentNode*>(item);
        if ( node ){
            nodes.push_back(node);
        }
    }
    return nodes ;
}

SocketWidget* GraphPanel::findSocket(
    SocketType type,
    std::optional<int> componentId,
    std::optional<size_t> idx,
    std::optional<ParameterType> param
){
    GraphNode* w = nullptr ;

    
    if ( !componentId.has_value() ){ 
        // missing component id means its a hardware socket
        if ( type == SocketType::SignalInbound ){
            w = audioOut_ ;
            return getNodeSocket(audioOut_, type, static_cast<size_t>(0));
        } else if ( type == SocketType::MidiOutbound ){
            return getNodeSocket(midiIn_, type);
        } 
    } else {
        w = getNode(componentId.value());
    }

    if ( !w ){ 
        qWarning() << "Could not find socket.";
        return nullptr ;
    }

    switch(type){
    case SocketType::ModulationInbound:
    {
        if ( !param.has_value() ){
            qWarning() << "Inbound Modulation specified without defining parameter. Cannot find socket.";
            return nullptr ;
        }
        ParameterType mp = static_cast<ParameterType>(param.value());
        return getNodeSocket(w, type, mp);
    }
    case SocketType::SignalInbound:
    case SocketType::SignalOutbound:
        if ( !idx.has_value() ){
            qWarning() << "Signal Socket Type selected but no index specified. Invalid socket requested." ;
            return nullptr ;
        }
        return getNodeSocket(w, type, idx.value());
    case SocketType::MidiInbound:
    case SocketType::ModulationOutbound:
    case SocketType::MidiOutbound:
        return getNodeSocket(w, type);
    default:
        qWarning() << "invalid socket type specified. Exiting.";
        return nullptr ;
    }
}

SocketWidget* GraphPanel::findSocketAt(const QPointF& scenePos){
    auto items = scene_->items(scenePos);
    for ( auto item : items ){
        if ( SocketWidget* socket = dynamic_cast<SocketWidget*>(item) ){
            return socket ;
        }
    }
    return nullptr ;
}

void GraphPanel::keyPressEvent(QKeyEvent* event){
    switch (event->key()){
        case Qt::Key_Delete:
        case Qt::Key_Backspace:
            for ( const auto c : getSelectedComponents() ){
                componentManager_->requestRemoveComponent(c->getModel()->getId());
            }
            break ;
        case Qt::Key_Escape:
            connectionRenderer_->cancelDrag();
            break ;
        default:
            QGraphicsView::keyPressEvent(event);
    }
}

void GraphPanel::mouseMoveEvent(QMouseEvent* event){
    QPointF scenePos = mapToScene(event->pos());
    SocketWidget* w = findSocketAt(scenePos);

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
        connectionRenderer_->updateDrag(scenePos);

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
        if ( SocketWidget* w = findSocketAt(scenePos) ){
            isDraggingConnection_ = true ;
            connectionRenderer_->startDrag(w);
            event->accept();
            return ;
        }
    }

    if ( event->button() == Qt::RightButton ){
        // TODO: maybe if we right click a module we #include "meta/ComponentDescriptor.hpp"can do some stuff...
    }

    QGraphicsView::mousePressEvent(event); // pass event through
}

void GraphPanel::mouseDoubleClickEvent(QMouseEvent* event){
    QPointF scenePos = mapToScene(event->pos());

    // Launch ComponentNode
    QGraphicsItem* item = scene()->itemAt(scenePos, transform());
    while (item){
        if ( ComponentNode* w = dynamic_cast<ComponentNode*>(item)){ 
            componentDoubleClicked(w);
        } 
        item = item->parentItem();
    }
}

void GraphPanel::mouseReleaseEvent(QMouseEvent* event){
    QPointF scenePos = mapToScene(event->pos());

    if (event->button() == Qt::LeftButton && isDraggingConnection_ ) {
        isDraggingConnection_ = false;
        connectionRenderer_->finishDrag(scenePos);
        event->accept();
        return ;
    }

    QGraphicsView::mouseReleaseEvent(event);
}

void GraphPanel::contextMenuEvent(QContextMenuEvent *event){
    QPointF scenePos = mapToScene(event->pos());

    // right clicking on a socket
    if ( SocketWidget* w = findSocketAt(scenePos) ){
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
        scale(Theme::GRAPH_WHEEL_SCALE_FACTOR, Theme::GRAPH_WHEEL_SCALE_FACTOR);
    } else {
        scale( 1.0 / Theme::GRAPH_WHEEL_SCALE_FACTOR, 1.0 / Theme::GRAPH_WHEEL_SCALE_FACTOR );
    }
}

void GraphPanel::drawBackground(QPainter* painter, const QRectF& rect){
    // Draw Grid
    painter->setPen(QPen(Theme::GRAPH_GRID_COLOR, 1));

    qreal left = int(rect.left()) - (int(rect.left()) % int(Theme::GRAPH_GRID_SIZE));
    qreal top = int(rect.top()) - (int(rect.top()) % int(Theme::GRAPH_GRID_SIZE));

    QVarLengthArray<QLineF, 100> lines ;

    for (qreal x = left; x < rect.right(); x+= Theme::GRAPH_GRID_SIZE){
        lines.append(QLineF(x, rect.top(), x, rect.bottom()));
    }

    for (qreal y = top; y < rect.bottom(); y += Theme::GRAPH_GRID_SIZE){
        lines.append(QLineF(rect.left(), y, rect.right(), y));
    }

    painter->drawLines(lines.data(), lines.size());

}

void GraphPanel::onApiDataReceived(const QJsonObject& json){
    QString action = json["action"].toString();

    if ( action == "load_configuration" ){
        if ( json["status"] == "success"){
            loadPositions(json);
        }
    }
}

void GraphPanel::onComponentSelected(ComponentType type){
    componentManager_->requestAddComponent(type);
}

void GraphPanel::onComponentAdded(int componentId, ComponentType type){
    auto m = componentManager_->getModel(componentId);

    if ( !m ){
        qWarning() << "cannot create component node. Model not found";
        return ;
    }

    auto n = new ComponentNode(m);
    nodes_.push_back(n);

    connect(n, &GraphNode::needsZUpdate, this, &GraphPanel::onNodeZUpdate);
    connect(n, &GraphNode::positionChanged, connectionRenderer_, &ConnectionRenderer::onNodePositionChanged);

    scene_->addItem(n);
    for ( auto s : n->getSockets() ){
        scene_->addItem(s);
    }

    n->setPos(0,0); // TODO: dynamically place the module somewhere currently empty on the scene
}

void GraphPanel::onComponentRemoved(int componentId){
    auto n = getNode(componentId);
    if ( !n ){
        qWarning() << "requested removal of component id which does not exist. id:" << componentId ;
        return ;
    }

    nodes_.erase(std::remove(nodes_.begin(), nodes_.end(), n), nodes_.end());
    scene_->removeItem(n);
    n->deleteLater();
}

void GraphPanel::componentDoubleClicked(ComponentNode* widget){
    int id = widget->getModel()->getId();

    componentManager_->showEditor(id);
}

void GraphPanel::onNodeZUpdate(){
    GraphNode* node = dynamic_cast<GraphNode*>(sender());
    int maxZ = 0 ;
    for ( auto n : nodes_ ){
        if ( n != node && n->zValue() > maxZ ){
            maxZ = n->zValue();
        }
    }

    if ( maxZ != 0 && node->zValue() == maxZ ) return ;

    // nodes in front of cables in front of sockets
    node->setZValue( maxZ + 1 );

    auto cables = connectionRenderer_->getNodeConnections(node);
    for ( auto* cable : cables ){
        cable->setZValue( maxZ  + 0.9 ); 
    }

    auto sockets = node->getSockets();
    for ( auto* socket: sockets){
        socket->setZValue(maxZ + 0.8);
    }
}