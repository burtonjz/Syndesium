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

#include "core/Synth.hpp"
#include "core/GraphPanel.hpp"
#include "core/ApiClient.hpp"
#include "meta/ComponentRegistry.hpp"
#include "types/ComponentType.hpp"

#include "ui_Synth.h"

#include <QJsonArray>
#include <QJsonObject>
#include <QJsonValue>
#include <QJsonDocument>
#include <QStandardItemModel>
#include <QStandardItem>
#include <qcombobox.h>
#include <qobject.h>

Synth::Synth(ModuleContext ctx, QWidget* parent):
    QMainWindow(parent),
    ui_(new Ui::MainWindow),
    ctx_(ctx),
    graph_(nullptr),
    setup_(nullptr)
{
    // API connections
    connect(ApiClient::instance(), &ApiClient::connected, this, &Synth::onApiConnected);
    
    ui_->setupUi(this);

    graph_ = new GraphPanel(this);
    ui_->graphPanelContainer->layout()->addWidget(graph_);

    // widget config
    ui_->addModuleBox->addItem("Add a Module...");
    ui_->addModuleBox->setCurrentIndex(0);
    ui_->addModuleBox->insertSeparator(1);

    ui_->addModulatorBox->addItem("Add a Modulator...");
    ui_->addModulatorBox->setCurrentIndex(0);
    ui_->addModulatorBox->insertSeparator(1);

    ui_->addMidiComponentBox->addItem("Add a Midi Component...");
    ui_->addMidiComponentBox->setCurrentIndex(0);
    ui_->addMidiComponentBox->insertSeparator(1);

    // force that first index to be a label and not enabled to select
    QStandardItemModel* m = qobject_cast<QStandardItemModel*>(ui_->addModuleBox->model());
    if(m){
        QStandardItem* item = m->item(0);
        if (item) item->setEnabled(false);
    }

    m = qobject_cast<QStandardItemModel*>(ui_->addModulatorBox->model());
    if(m){
        QStandardItem* item = m->item(0);
        if (item) item->setEnabled(false);
    }

    m = qobject_cast<QStandardItemModel*>(ui_->addMidiComponentBox->model());
    if(m){
        QStandardItem* item = m->item(0);
        if (item) item->setEnabled(false);
    }

    // now loop through descriptors and add them to the appropriate box
    auto reg = ComponentRegistry::getAllComponentDescriptors();
    QString name ;
    int typ ;
    for ( auto item : reg ){
        name = QString::fromStdString(item.second.name);
        if ( item.second.isModule() ){
            typ = static_cast<int>(item.first);
            ui_->addModuleBox->addItem(name, typ);
        } 
        if ( item.second.isModulator() ){
            typ = static_cast<int>(item.first);
            ui_->addModulatorBox->addItem(name, typ);
        }
        if ( item.second.isMidiHandler() ){
            typ = static_cast<int>(item.first);
            ui_->addMidiComponentBox->addItem(name, typ);
        }
    }

    // connections
    connect(this, &Synth::engineStatusChanged, this, &Synth::onEngineStatusChange);
    connect(ui_->setupButton, &QPushButton::clicked, this, &Synth::onSetupButtonClicked);
    connect(ui_->startStopButton, &QPushButton::clicked, this, &Synth::onStartStopButtonClicked);
    connect(ApiClient::instance(), &ApiClient::dataReceived, this, &Synth::onApiDataReceived);
    connect(ui_->addModuleBox, QOverload<int>::of(&QComboBox::currentIndexChanged), 
        this, [this](int index){ onComponentAdded(index); });
    connect(ui_->addModulatorBox, QOverload<int>::of(&QComboBox::currentIndexChanged), 
        this, [this](int index){ onComponentAdded(index); });
    connect(ui_->addMidiComponentBox, QOverload<int>::of(&QComboBox::currentIndexChanged),
        this, [this](int index){ onComponentAdded(index); });
    connect(this, &Synth::componentAdded, graph_, &GraphPanel::onComponentAdded);
}

Synth::~Synth(){
    delete ui_ ;
}

void Synth::onApiConnected(){

}

void Synth::onApiDataReceived(const QJsonObject &json){
    QString action = json["action"].toString();

    if ( action == "set_state" ){
        QString state = json["state"].toString();
        if ( json["status"] != "success"){
            qDebug() << "request to set state was unsuccessful." ;
            return ;
        }
        if (  state == "stop" ){
            emit engineStatusChanged(false);
        } else if ( state == "run" ) {
            emit engineStatusChanged(true);
        } else {
            qDebug() << "invalid state received from set_state" << state ;
        }
        return ;
    }

}

void Synth::onSetupButtonClicked(){
    qDebug() << "launching setup window" ;
    if ( !setup_ ){
        qDebug() << "Setup window does not exist, creating widget..." ;
        ModuleContext ctx = {ctx_.state, "Setup"};
        setup_ = new Setup(ctx, this) ;
        setup_->show();
    } else {
        qDebug() << "Setup window already exists, displaying..." ;
        if (!setup_->isVisible()){
            setup_->show();
        }
    }
}

void Synth::onStartStopButtonClicked(){
    if ( ctx_.state->isRunning()){
        QJsonObject obj ;
        obj["action"] = "set_state" ;
        obj["state"] = "stop" ;
        ApiClient::instance()->sendMessage(obj);
    } else {
        QJsonObject obj ;
        obj["action"] = "set_state" ;
        obj["state"] = "run" ;
        ApiClient::instance()->sendMessage(obj);
    }
}

void Synth::onEngineStatusChange(bool status){
    qDebug() << "engine status changed. Setting new button text" ;
    ctx_.state->setRunning(status);
    if (status){
        ui_->startStopButton->setText("Stop");
    } else {
        ui_->startStopButton->setText("Play");
    }
}

void Synth::onComponentAdded(int index){
    if ( index == 0 ) return ;

    auto cbox = dynamic_cast<QComboBox*>(sender());

    ComponentType typ = static_cast<ComponentType>(cbox->itemData(index).toInt());
    cbox->setCurrentIndex(0); 
    emit componentAdded(typ);
}