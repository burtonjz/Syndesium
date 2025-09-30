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
#include "meta/ComponentDescriptor.hpp"
#include "meta/ComponentRegistry.hpp"
#include "widgets/SocketContainerWidget.hpp"
#include "widgets/ComponentWidget.hpp"
#include "types/ModuleType.hpp"

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

#include <QJsonObject>
#include <qdebug.h>
#include <qevent.h>
#include <qgraphicsitem.h>
#include <qgraphicsscene.h>
#include <qjsonobject.h>
#include <qnamespace.h>
#include <qvarlengtharray.h>

GraphPanel::GraphPanel(QWidget* parent):
    QGraphicsView(parent)
{
    setupScene();
    addMidiInput();
    addAudioOutput();

    setFocusPolicy(Qt::StrongFocus);
    setEnabled(true);

    // connections
    connect(ApiClient::instance(), &ApiClient::dataReceived, this, &GraphPanel::onApiDataReceived);

    
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

void GraphPanel::addComponent(int id, ComponentType type){
    auto* module = new ComponentWidget(id, type);
    auto* detail = new ModuleDetailWidget(id,type);

    widgets_.push_back(module);
    details_.push_back(detail);

    scene_->addItem(module);
    module->setPos(0,0); // TODO: dynamically place the module somewhere currently empty on the scene

    connectWidgetSignals(module);
    
    qDebug() << "Created module:" << module->getName() << "at position:" << module->pos() ;
}

void GraphPanel::addAudioOutput(){
    auto* container = new SocketContainerWidget("Audio Ouput Device");
    container->createSockets({{SocketType::SignalInput, "Audio In"}});
    scene_->addItem(container);

    connectWidgetSignals(container);
    widgets_.push_back(container);
    qDebug() << "Created Audio Output Device Widget:" << container->getName() << "at position:" << container->pos() ;
}

void GraphPanel::addMidiInput(){
    auto* container = new SocketContainerWidget("MIDI Input Device");
    container->createSockets({{SocketType::MidiOutput, "MIDI Out"}});
    scene_->addItem(container);

    connectWidgetSignals(container);
    widgets_.push_back(container);
    qDebug() << "Created Midi Input Device Widget:" << container->getName() << "at position:" << container->pos() ;
}

void GraphPanel::connectWidgetSignals(SocketContainerWidget* widget){
    for (SocketWidget* socket : widget->getSockets() ){
        connect(socket, &SocketWidget::connectionStarted, this, &GraphPanel::onConnectionStarted);
        connect(socket, &SocketWidget::connectionDragging, this, &GraphPanel::onConnectionDragging);
        connect(socket, &SocketWidget::connectionEnded, this, &GraphPanel::onConnectionEnded);
    }
}

void GraphPanel::deleteSelectedModules(){
    QList<QGraphicsItem*> selectedItems = scene_->selectedItems() ;

    for ( QGraphicsItem* item: selectedItems ){
        if ( ComponentWidget* module = qgraphicsitem_cast<ComponentWidget*>(item) ){
            connectionManager_->removeAllConnections(module);
            auto it = std::find(widgets_.begin(), widgets_.end(), module);
            if ( it != widgets_.end() ) widgets_.erase(it) ;

            scene_->removeItem(module);
            module->deleteLater();

            qDebug() << "Deleted module:" << module->getComponentDescriptor().name ;
        }
    }
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

void GraphPanel::mousePressEvent(QMouseEvent* event){
    if ( event->button() == Qt::RightButton ){
        // TODO: maybe if we right click a module we can do some stuff...
    }

    QGraphicsView::mousePressEvent(event); // pass event through
}

void GraphPanel::mouseDoubleClickEvent(QMouseEvent* event){
    QPointF scenePos = mapToScene(event->pos());

    QGraphicsItem* item = scene()->itemAt(scenePos, transform());

    if ( ComponentWidget* w = dynamic_cast<ComponentWidget*>(item)){
        qDebug() << "Component Clicked!";
        onComponentDoubleClicked(w);
    }
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
    painter->setPen(QPen(GRAPH_GRID_COLOR, 1));

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
    obj["action"] = "add_component" ;
    obj["name"] = QString::fromStdString(ComponentRegistry::getComponentDescriptor(type).name) ;
    if ( type.isModule() ){
        obj["is_module"] = true ;
        obj["type"] = static_cast<int>(type.getModuleType());
    } else {
        obj["is_module"] = false ;
        obj["type"] = static_cast<int>(type.getModulatorType());
    }
    ApiClient::instance()->sendMessage(obj); 
}

void GraphPanel::onApiDataReceived(const QJsonObject& json){
    QString action = json["action"].toString();

    if ( action == "add_component" ){
        if ( json["status"] != "success" ){
            qDebug() << "module was not successfully added." ;
            return ;
        }

        int id = json["component_id"].toInt();
        if ( json["is_module"].toBool() ){
            ModuleType type = static_cast<ModuleType>(json["type"].toInt());
            addComponent(id,type);
        } else {
            ModulatorType type = static_cast<ModulatorType>(json["type"].toInt());
            addComponent(id,type);
        }
    }
}

void GraphPanel::onComponentDoubleClicked(ComponentWidget* widget){
    int id = widget->getID();
    ComponentType type = widget->getComponentDescriptor().type ;

    for ( auto d : details_ ){
        if( d->getID() == id && d->getType() == type ){
            d->show();
            d->raise();
            d->activateWindow();
            return ;
        }
    }

    qDebug() << "No detail component found.";
}

void GraphPanel::onConnectionStarted(SocketWidget* socket){
    qDebug() << "Connection started from:" << socket->getParent()->getName() << "-" << socket->getName() ;
    connectionManager_->startConnection(socket);
}

void GraphPanel::onConnectionDragging(SocketWidget* socket, QPointF scenePos){
    Q_UNUSED(socket);
    connectionManager_->updateDragConnection(scenePos);

    // manually show tool tip if hovering
    SocketWidget* w = connectionManager_->findSocketAt(scenePos);
    
    if (w && !w->toolTip().isEmpty()){
        QToolTip::showText(QCursor::pos(), w->toolTip());
        return ;
    }

    // hide tool tip if no longer found
    QToolTip::hideText();
}

void GraphPanel::onConnectionEnded(SocketWidget* socket, QPointF scenePos){
    Q_UNUSED(socket);
    qDebug() << "Connection ended at:" << scenePos ;
    connectionManager_->finishConnection(scenePos);
}