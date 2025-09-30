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

#include "widgets/Oscillator.hpp"
#include "ui_Oscillator.h"
#include <QJsonObject>
#include <QJsonValue>
#include <QJsonArray>
#include <qcombobox.h>
#include <qjsonobject.h>

Oscillator::Oscillator(ApiClient* apiClient, int moduleId, QWidget *parent): 
    QWidget(parent), 
    ui(new Ui::Oscillator),
    client(apiClient),
    id(moduleId)
{
    ui->setupUi(this);

    connect(client, &ApiClient::dataReceived, this, &Oscillator::onApiDataReceived);
    connect(ui->comboWaveform, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &Oscillator::onWaveformComboBoxChanged);
}

Oscillator::~Oscillator()
{
    delete ui;
}

void Oscillator::initialize(){
    // send initialization api calls
    QJsonObject obj ;
    obj["action"] = "get_waveforms" ;
    client->sendMessage(obj);

    obj["action"] = "get_oscillator_waveform" ;
    obj["module_id"] = id ; 
    client->sendMessage(obj);
}

void Oscillator::onApiDataReceived(const QJsonObject &json){
    QString action = json["action"].toString();
    QJsonValue data = json.value("data");

    if ( action == "get_waveforms" ){
        if (!data.isArray()){
            qWarning() << "Expected 'data' to be a JSON array, but is" << data ;
            return ;
        }
        QJsonArray dataArray = data.toArray();

        ui->comboWaveform->clear();

        for ( const QJsonValue &wf : dataArray ){
            QString waveform = wf.toString() ;
            ui->comboWaveform->addItem(waveform);
        }
    }

    if ( action == "get_oscillator_waveform" ){
        QString waveform = data.toString();
        ui->comboWaveform->setCurrentText(waveform);
    }
}

void Oscillator::onWaveformComboBoxChanged(int index){
    QString waveform = ui->comboWaveform->currentText() ;

    QJsonObject obj ;
    obj["action"] = "set_oscillator_waveform" ;
    obj["module_id"] = id   ;
    obj["data"] = waveform ;
    client->sendMessage(obj);


}