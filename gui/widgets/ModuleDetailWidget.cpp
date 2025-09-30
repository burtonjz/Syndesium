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

#include "widgets/ModuleDetailWidget.hpp"
#include "meta/ComponentDescriptor.hpp"
#include "meta/ComponentRegistry.hpp"
#include "types/ParameterType.hpp"
#include "types/Waveform.hpp"
#include <qboxlayout.h>
#include <qevent.h>
#include <qobject.h>
#include <qpushbutton.h>

ModuleDetailWidget::ModuleDetailWidget(int id, ComponentType type, QWidget* parent):
    QWidget(parent),
    componentId_(id),
    descriptor_(ComponentRegistry::getComponentDescriptor(type))
{
    for ( auto p: descriptor_.controllableParameters ){
        createParameter(p);
    }

    setupLayout();
}

int ModuleDetailWidget::getID() const {
    return componentId_ ;
}

ComponentType ModuleDetailWidget::getType() const {
    return descriptor_.type ;
}

void ModuleDetailWidget::createParameter(ParameterType p){
    switch(p){
    case ParameterType::WAVEFORM:
        createWaveformWidget();
        break ;
    case ParameterType::FILTER_TYPE:
        // parameterWidgets_[p] = new QComboBox(this);
        break ;
    default:
        // most things will just be dials
        createSpinWidget(p);
    }
}

void ModuleDetailWidget::createWaveformWidget(){
    // populate combo box with available waveforms
    auto w = new QComboBox(this);
    for ( auto wf : Waveform::getWaveforms()){
        Waveform wave = Waveform(wf);
        w->addItem(QString::fromStdString(wave.toString()),wave.to_uint8());
    }

    // set the current waveform to default
    int index = w->findData(parameterDefaults[static_cast<uint8_t>(ParameterType::WAVEFORM)]);
    if ( index != -1 ){
        w->setCurrentIndex(index);
    } 

    parameterWidgets_[ParameterType::WAVEFORM] = w ;
}

void ModuleDetailWidget::createSpinWidget(ParameterType p){
    auto w = new QSpinBox(this);
    float defaultValue = parameterDefaults[static_cast<uint8_t>(p)];
    auto limit = parameterLimits[static_cast<uint8_t>(p)];
    w->setRange(limit.first, limit.second);
    w->setSingleStep((limit.second - limit.first) / 100.0 );
    w->setValue(defaultValue);
}

void ModuleDetailWidget::setupLayout(){
    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    QHBoxLayout* buttonLayout = new QHBoxLayout(this);
    QHBoxLayout* parameterLayout = new QHBoxLayout(this);

    // parameters
    parameterLayout->setSpacing(PARAMETER_WIDGET_SPACING);

    for ( auto it = parameterWidgets_.begin(); it != parameterWidgets_.end(); ++it){
        ParameterType p = it.key(); 
        QWidget* w = it.value();

        QVBoxLayout* column = new QVBoxLayout();

        QString labelText = QString::fromStdString(parameter2String(p));
        QLabel* label = new QLabel(labelText, this);

        column->addWidget(label);
        column->addWidget(w);
        parameterLayout->addLayout(column);
    }

    // buttons
    closeButton_ = new QPushButton("Close",this);
    resetButton_ = new QPushButton("Reset", this);
    buttonLayout->addWidget(closeButton_);
    buttonLayout->addWidget(resetButton_);

    mainLayout->addLayout(parameterLayout);
    mainLayout->addLayout(buttonLayout);

    // resize this widget to fit all parameters
    int nParameters = parameterWidgets_.size();
    int width = (nParameters * PARAMETER_WIDGET_WIDTH) + ((nParameters - 1) * PARAMETER_WIDGET_SPACING) + MODULE_DETAIL_MARGINS ;
    resize(width, sizeHint().height());
}

void ModuleDetailWidget::closeEvent(QCloseEvent* event){
    event->ignore();
    hide();
    emit widgetClosed();
}

void ModuleDetailWidget::onCloseButtonClicked(){
    hide();
    emit widgetClosed();
}

