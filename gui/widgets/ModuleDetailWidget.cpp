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
#include "types/ComponentType.hpp"
#include "types/ParameterType.hpp"
#include "types/Waveform.hpp"
#include "core/ApiClient.hpp"

#include <QJsonObject>
#include <QEvent>
#include <QCloseEvent>

ModuleDetailWidget::ModuleDetailWidget(int id, ComponentType type, QWidget* parent):
    QWidget(parent),
    componentId_(id),
    descriptor_(ComponentRegistry::getComponentDescriptor(type))
{
    setWindowTitle(QString::fromStdString(descriptor_.name));
    for ( auto p: descriptor_.controllableParameters ){
        createParameter(p);
    }

    setupLayout();

    connect(this, &ModuleDetailWidget::parameterChanged, &ModuleDetailWidget::onParameterChanged);
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
    int index = w->findData(GET_PARAMETER_TRAIT_MEMBER(ParameterType::WAVEFORM, defaultValue));
    if ( index != -1 ){
        w->setCurrentIndex(index);
    } 

    parameterWidgets_[ParameterType::WAVEFORM] = w ;
    connect(w, &QComboBox::currentTextChanged, this, &ModuleDetailWidget::onValueChange);
}

void ModuleDetailWidget::createSpinWidget(ParameterType p){
    auto w = new QDoubleSpinBox(this);
    
    auto step = GET_PARAMETER_TRAIT_MEMBER(p, uiStepPrecision);
    w->setRange(GET_PARAMETER_TRAIT_MEMBER(p, minimum), GET_PARAMETER_TRAIT_MEMBER(p, maximum));
    w->setSingleStep(step);
    w->setValue(GET_PARAMETER_TRAIT_MEMBER(p, defaultValue));

    parameterWidgets_[p] = w ;
    connect(w, &QDoubleSpinBox::valueChanged, this, &ModuleDetailWidget::onValueChange);
}

void ModuleDetailWidget::setupLayout(){
    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    QHBoxLayout* buttonLayout = new QHBoxLayout();
    QHBoxLayout* parameterLayout = new QHBoxLayout();

    // parameters
    parameterLayout->setSpacing(PARAMETER_WIDGET_SPACING);

    for ( auto it = parameterWidgets_.begin(); it != parameterWidgets_.end(); ++it){
        ParameterType p = it.key(); 
        QWidget* w = it.value();
        QVBoxLayout* column = new QVBoxLayout();
        std::string name = GET_PARAMETER_TRAIT_MEMBER(p, name);
        QString labelText = QString::fromStdString(name);
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

void ModuleDetailWidget::onValueChange(){
    auto widget = dynamic_cast<QWidget*>(sender());
    
    auto it = std::find(parameterWidgets_.begin(), parameterWidgets_.end(), widget);
    if ( it == parameterWidgets_.end() ) return ;
    
    ParameterType p = it.key();
    ParameterValue v ;
    if ( auto cb = dynamic_cast<QComboBox*>(it.value())){
        v = cb->itemData(cb->currentIndex()).value<uint8_t>();
    } else if ( auto sb = dynamic_cast<QDoubleSpinBox*>(it.value())){
        v = sb->value();
    }

    emit parameterChanged(componentId_, descriptor_, p, v);
    
}

void ModuleDetailWidget::onParameterChanged(int componentId, ComponentDescriptor descriptor, ParameterType p, ParameterValue value){
    QJsonObject obj ;
    obj["action"] = "set_component_parameter" ;
    obj["componentId"] = componentId ;
    obj["parameter"] = static_cast<int>(p);
    obj["value"] = QVariant::fromStdVariant(value).toJsonValue();
    obj["isModule"] = descriptor_.isModule() ;

    ApiClient::instance()->sendMessage(obj); 
}