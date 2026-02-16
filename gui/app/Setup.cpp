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

#include "app/Setup.hpp"
#include "app/ModuleContext.hpp"
#include "api/ApiClient.hpp"
#include "ui_Setup.h"

#include <QJsonArray>
#include <QJsonObject>
#include <QJsonValue>
#include <QJsonDocument>

Setup::Setup(ModuleContext ctx, QWidget* parent): 
    QWidget(parent),
    ui_(new Ui::AudioMidiSetupWidget),
    ctx_(ctx)
{
    // create API connections
    connect(ApiClient::instance(), &ApiClient::dataReceived, this, &Setup::onApiDataReceived);

    // ask backend for relevant data
    QJsonObject obj ;
    obj["action"] = "get_audio_devices" ;
    ApiClient::instance()->sendMessage(obj);
    obj["action"] = "get_midi_devices" ;
    ApiClient::instance()->sendMessage(obj); 

    ui_->setupUi(this);

    // connections
    connect(ui_->pushButtonSubmit, &QPushButton::clicked, this, &Setup::onSetupSubmit);
    connect(ctx_.state, &StateManager::setupCompleted, this, &Setup::onSetupCompleted);
}

Setup::~Setup()
{
    delete ui_;
}

void Setup::populateSetupComboBox(QComboBox* box, QJsonValue data){
    if (!data.isArray()){
        qWarning() << "Expected 'data' to be a JSON array, but is" << data ;
        return ;
    }

    QJsonArray dataArray = data.toArray();

    box->clear();

    for ( const QJsonValue &dev : dataArray ){
        if ( !dev.isArray()  ){
            qWarning() << "Expected array elements to be arrays, but object is " << dev ;
        }
        QJsonArray info = dev.toArray();

        int deviceID = info.at(0).toInt();
        QString deviceName = info.at(1).toString();

        QString displayText = QString("(%1) %2").arg(deviceID).arg(deviceName);
        box->addItem(displayText, deviceID);
    }
}


void Setup::onApiDataReceived(const QJsonObject &json){
    QString action = json["action"].toString();

    if ( action == "get_audio_devices" ){
        populateSetupComboBox(ui_->comboAudioDevice, json.value("data"));
        return ;
    }

    if ( action == "get_midi_devices" ){
        populateSetupComboBox(ui_->comboMidiDevice, json.value("data"));
        return ;
    }

    if ( action == "set_audio_device" ){
        QString status = json["status"].toString();
        qDebug() << "set_audio_device return state:" << status ;
        if ( status == "success" ){
            ctx_.state->setSetupAudioComplete(true);
        }
        return ;
    }

    if ( action == "set_midi_device" ){
        QString status = json["status"].toString();
        qDebug() << "set_midi_device return state:" << status ;
        if ( status == "success" ){
            ctx_.state->setSetupMidiComplete(true);
        }
        return ;
    }
}

void Setup::onSetupSubmit(){
    qInfo() << "Setup submit button clicked." ;
    QJsonObject obj ;
    obj["action"] = "set_audio_device" ;
    obj["device_id"] = ui_->comboAudioDevice->currentData().toInt();
    ApiClient::instance()->sendMessage(obj);

    obj["action"] = "set_midi_device" ;
    obj["device_id"] = ui_->comboMidiDevice->currentData().toInt();
    ApiClient::instance()->sendMessage(obj);
}

void Setup::onSetupCompleted(){
    qInfo() << "setup completed." ;
    QJsonObject obj ; 
    emit setupCompleted();
    close();
}
