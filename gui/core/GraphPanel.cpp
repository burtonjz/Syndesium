#include "core/GraphPanel.hpp"
#include "core/ApiClient.hpp"
#include "modules/ModuleWidget.hpp"
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

#include <QJsonObject>
#include <qdebug.h>
#include <qevent.h>
#include <qgraphicsitem.h>
#include <qgraphicsscene.h>
#include <qjsonobject.h>
#include <qnamespace.h>
#include <qvarlengtharray.h>

GraphPanel::GraphPanel(ApiClient* client, QWidget* parent):
    QGraphicsView(parent),
    apiClient_(client)
{
    setupScene();

    setFocusPolicy(Qt::StrongFocus);
    setEnabled(true);

    // connections
    connect(apiClient_, &ApiClient::dataReceived, this, &GraphPanel::onApiDataReceived);
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

void GraphPanel::addModule(int id, ModuleType type){
    auto* module = new ModuleWidget(id, type);
    scene_->addItem(module);
    module->setPos(0,0); // TODO: dynamically place the module somewhere currently empty on the scene

    connectModuleSignals(module);
    qDebug() << "Created module:" << module->getModuleDescriptor().name << "at position:" << module->pos() ;
}

void GraphPanel::connectModuleSignals(ModuleWidget* module){
    connect(module, &ModuleWidget::moduleDoubleClicked, this, &GraphPanel::onModuleDoubleClicked);
    for (SocketWidget* socket : module->getSockets() ){
        connect(socket, &SocketWidget::connectionStarted, this, &GraphPanel::onConnectionStarted);
        connect(socket, &SocketWidget::connectionDragging, this, &GraphPanel::onConnectionDragging);
        connect(socket, &SocketWidget::connectionEnded, this, &GraphPanel::onConnectionEnded);
    }
}

void GraphPanel::deleteSelectedModules(){
    QList<QGraphicsItem*> selectedItems = scene_->selectedItems() ;

    for ( QGraphicsItem* item: selectedItems ){
        if ( ModuleWidget* module = qgraphicsitem_cast<ModuleWidget*>(item) ){
            connectionManager_->removeAllConnections(module);
            modules_.removeOne(module);
            scene_->removeItem(module);
            module->deleteLater();

            qDebug() << "Deleted module:" << module->getModuleDescriptor().name ;
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

void GraphPanel::onModuleAdded(ModuleType type){
    QJsonObject obj ;
    obj["action"] = "add_module" ;
    obj["name"] = "placeholder" ;
    obj["type"] = static_cast<int>(type);
    apiClient_->sendMessage(obj); 
}

void GraphPanel::onApiDataReceived(const QJsonObject& json){
    QString action = json["action"].toString();

    if ( action == "add_module" ){
        if ( json["status"] != "success" ){
            qDebug() << "module was not successfully added." ;
            return ;
        }
        ModuleType type = static_cast<ModuleType>(json["type"].toInt());
        int id = json["module_id"].toInt();
        addModule(id,type);
    }
}

void GraphPanel::onModuleDoubleClicked(ModuleWidget* module){
    // TODO -- use this to launch module specific control UIs
    qDebug() << "Module double-clicked:" << module->getModuleDescriptor().name ;
}

void GraphPanel::onConnectionStarted(SocketWidget* socket){
    qDebug() << "Connection started from:" << socket->getParent()->getModuleDescriptor().name << "-" << socket->getName() ;
    connectionManager_->startConnection(socket);
}

void GraphPanel::onConnectionDragging(SocketWidget* socket, QPointF scenePos){
    Q_UNUSED(socket);
    connectionManager_->updateDragConnection(scenePos);
}

void GraphPanel::onConnectionEnded(SocketWidget* socket, QPointF scenePos){
    Q_UNUSED(socket);
    qDebug() << "Connection ended at:" << scenePos ;
    connectionManager_->finishConnection(scenePos);
}