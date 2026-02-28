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
#include "graphics/GroupNode.hpp"
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
    connect(
        componentManager_, &ComponentManager::componentGroupUpdated,
        this, &GraphPanel::onComponentGroupUpdate
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
    audioOut_->createSockets({{
        .type = SocketType::SignalInbound, 
        .name = "Audio In",
        .idx  = 0
    }});
    
    audioOut_->addToScene(scene_);
    nodes_.push_back(audioOut_);
    
    connect(audioOut_, &GraphNode::needsZUpdate, this, &GraphPanel::onNodeZUpdate);
    connect(audioOut_, &GraphNode::positionChanged, connectionRenderer_, &::ConnectionRenderer::onNodePositionChanged);

    qDebug() << "Created Audio Output Device Widget:" << audioOut_->getName() << "at position:" << audioOut_->pos() ;
}

void GraphPanel::addMidiInput(){
    midiIn_ = new GraphNode("MIDI Input Device");
    midiIn_->createSockets({{
        .type = SocketType::MidiOutbound, 
        .name = "MIDI Out"
    }});

    midiIn_->addToScene(scene_);
    nodes_.push_back(midiIn_);
    
    connect(midiIn_, &GraphNode::needsZUpdate, this, &GraphPanel::onNodeZUpdate);
    connect(midiIn_, &GraphNode::positionChanged, connectionRenderer_, &ConnectionRenderer::onNodePositionChanged);

    qDebug() << "Created Midi Input Device Widget:" << midiIn_->getName() << "at position:" << midiIn_->pos() ;
}

GraphNode* GraphPanel::getVisibleNode(int componentId) const {
    for ( auto n : nodes_ ){
        auto cNode = dynamic_cast<ComponentNode*>(n);
        if ( cNode && cNode->isVisible() ){
            if ( cNode->getModel()->getId() == componentId ){
                return cNode ;
            }
        }
        auto gNode = dynamic_cast<GroupNode*>(n);
        if ( gNode && gNode->contains(componentId) ){
            return gNode ;
        }
    }
    return nullptr ;
}

ComponentNode* GraphPanel::getComponentNode(int componentId) const {
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

GroupNode* GraphPanel::getGroupNode(int groupId) const {
    for ( auto n : nodes_ ){
        auto gNode = dynamic_cast<GroupNode*>(n);
        if ( gNode ){
            if ( gNode->getId() == groupId ){
                return gNode ;
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
        
        auto n = getComponentNode(componentID);
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
        if ( s->getSpec().type == t ){
            if ( t == SocketType::ModulationInbound ){
                ParameterType p = std::get<ParameterType>(selector);
                if ( p == parameterFromString(s->getSpec().name.toStdString())){
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

std::vector<GroupNode*> GraphPanel::getSelectedGroups() const {
    auto selectedItems = scene_->selectedItems() ;
    std::vector<GroupNode*> nodes ;

    for ( QGraphicsItem* item: selectedItems ){
        GroupNode* node = dynamic_cast<GroupNode*>(item);
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
        w = getVisibleNode(componentId.value());
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
        case Qt::Key_G:
            if ( event->modifiers() & Qt::ControlModifier ){
                handleGroupEvent();
            }
            break ;
        case Qt::Key_U:
            if ( event->modifiers() & Qt::ControlModifier ){
                handleUngroupEvent();
            }
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

    // Launch ComponentEditor
    QGraphicsItem* item = scene()->itemAt(scenePos, transform());
    while (item){
        if ( GraphNode* w = dynamic_cast<GraphNode*>(item)){ 
            graphNodeDoubleClicked(w);
            return ;
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

    n->addToScene(scene_);

    n->setPos(0,0); // TODO: dynamically place the module somewhere currently empty on the scene
}

void GraphPanel::onComponentRemoved(int componentId){
    auto n = getComponentNode(componentId);
    if ( !n ){
        qWarning() << "requested removal of component id which does not exist. id:" << componentId ;
        return ;
    }

    nodes_.erase(std::remove(nodes_.begin(), nodes_.end(), n), nodes_.end());
    scene_->removeItem(n);
    n->deleteLater();
}

void GraphPanel::graphNodeDoubleClicked(GraphNode* widget){
    if ( auto c = dynamic_cast<ComponentNode*>(widget) ){
        componentManager_->showEditor(c->getModel()->getId());
        return ;
    }

    if ( auto g = dynamic_cast<GroupNode*>(widget) ){
        componentManager_->showGroupEditor(g->getId());
        return ;
    }
}

void GraphPanel::handleGroupEvent(){
    std::vector<int> groupIds ;
    std::vector<int> componentIds ;

    for ( const auto& g : getSelectedGroups() ){
        groupIds.push_back(g->getId());
    }

    for ( const auto& c: getSelectedComponents() ){
        componentIds.push_back(c->getModel()->getId());
    }

    if ( groupIds.size() == 0 && componentIds.size() == 0 ) return ;

    // case 1: no groups selected, create new group
    if ( groupIds.size() == 0 ){
        componentManager_->createGroup(componentIds);
    }

    // case 2: one group selected, add into group
    if ( groupIds.size() == 1 ){
        componentManager_->appendToGroup(groupIds[0], componentIds);
    }
}

void GraphPanel::handleUngroupEvent(){
    for ( const auto& g : getSelectedGroups() ){
        componentManager_->removeGroup(g->getId());
    }
}

void GraphPanel::onComponentGroupUpdate(int groupId, std::vector<int> componentIds){
    auto gNode = getGroupNode(groupId);

    // if it's a new group, create it
    if ( !gNode ){
        gNode = new GroupNode(groupId);
        nodes_.push_back(gNode);
        gNode->addToScene(scene_);
        //TODO make connection from group node to connection renderer
    }

    // reset and add componentIds
    gNode->removeAll();
    for ( const auto id : componentIds ){
        gNode->add(getComponentNode(id));
    }
    
    // if there are no component ids, then remove the group node
    if ( gNode->getNumComponents() == 0 ){
        nodes_.erase(std::remove(nodes_.begin(), nodes_.end(), gNode), nodes_.end());
        scene_->removeItem(gNode);
        gNode->deleteLater();
        return ;
    }
    
    // TODO: handle connection renderings to point to visible object
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