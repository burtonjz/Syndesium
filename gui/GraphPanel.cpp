#include "GraphPanel.hpp"
#include "ApiClient.hpp"
#include "modules/ModuleWidget.hpp"
#include "types/ModuleType.hpp"

#include <QJsonObject>
#include <qdebug.h>
#include <qjsonobject.h>

GraphPanel::GraphPanel(ApiClient* client, QWidget* parent):
    QGraphicsView(parent),
    apiClient_(client),
    scene_(new QGraphicsScene(this))
{
    setScene(scene_);
    setRenderHint(QPainter::Antialiasing);
    setDragMode(QGraphicsView::RubberBandDrag);
    setViewportUpdateMode(QGraphicsView::BoundingRectViewportUpdate);

    // scene_->setSceneRect(-500,  -500, 1000, 1000);

    // connections
    connect(apiClient_, &ApiClient::dataReceived, this, &GraphPanel::onApiDataReceived);
}

void GraphPanel::addModule(int id, ModuleType typ){
    auto* module = new ModuleWidget(id, typ);
    scene_->addItem(module);
    module->setPos(0,0); // TODO: dynamically place the module somewhere currently empty on the scene
}

void GraphPanel::onModuleAdded(ModuleType typ){
    QJsonObject obj ;
    obj["action"] = "add_module" ;
    obj["type"] = static_cast<int>(typ);
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
    }
}