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

#include "widgets/ComponentDetailWidget.hpp"
#include "meta/ComponentDescriptor.hpp"
#include "meta/ComponentRegistry.hpp"
#include "types/ComponentType.hpp"
#include "types/ParameterType.hpp"
#include "types/Waveform.hpp"
#include "types/FilterType.hpp"
#include "core/ApiClient.hpp"
#include "widgets/SequenceWidget.hpp"

#include <QJsonObject>
#include <QEvent>
#include <QCloseEvent>
#include <QBoxLayout>
#include <QScrollArea>

ComponentDetailWidget::ComponentDetailWidget(int id, ComponentType type, QWidget* parent):
    QWidget(parent),
    componentId_(id),
    descriptor_(ComponentRegistry::getComponentDescriptor(type)),
    sequenceWidget_(nullptr)
{
    setWindowTitle(QString::fromStdString(descriptor_.name));

    setWindowFlags(Qt::Tool | Qt::WindowStaysOnTopHint);
    setAttribute(Qt::WA_ShowWithoutActivating);

    for ( auto p: descriptor_.controllableParameters ){
        createParameter(p);
    }

    setupLayout();

    // handle parameter changes
    parameterChangedTimer_ = new QTimer(this);
    parameterChangedTimer_->setSingleShot(true);
    parameterChangedTimer_->setInterval(300);

    connect(parameterChangedTimer_, &QTimer::timeout, this, &ComponentDetailWidget::flushPendingChanges);
    
    // notify upstream of changes 
    modifiedTimer_ = new QTimer(this);
    modifiedTimer_->setSingleShot(true);
    modifiedTimer_->setInterval(300);

    connect(modifiedTimer_, &QTimer::timeout, this, &ComponentDetailWidget::wasModified);
    connect(this, &ComponentDetailWidget::parameterChanged, modifiedTimer_, qOverload<>(&QTimer::start)); 

    
}

int ComponentDetailWidget::getID() const {
    return componentId_ ;
}

ComponentType ComponentDetailWidget::getType() const {
    return descriptor_.type ;
}

void ComponentDetailWidget::createParameter(ParameterType p){
    switch(p){
    case ParameterType::WAVEFORM:
        createWaveformWidget();
        break ;
    case ParameterType::FILTER_TYPE:
        createFilterTypeWidget();
        break ;
    default:
        // most things will just be dials
        createSpinWidget(p);
    }
}

void ComponentDetailWidget::createWaveformWidget(){
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
    connect(w, &QComboBox::currentTextChanged, this, &ComponentDetailWidget::onValueChange);
}

void ComponentDetailWidget::createFilterTypeWidget(){
    auto w = new QComboBox(this);
    for ( auto ft : FilterType::getFilterTypes()){
        FilterType f = FilterType(ft);
        w->addItem(QString::fromStdString(f.toString()),f.to_uint8());
    }

    // set the current waveform to default
    int index = w->findData(GET_PARAMETER_TRAIT_MEMBER(ParameterType::FILTER_TYPE, defaultValue));
    if ( index != -1 ){
        w->setCurrentIndex(index);
    } 

    parameterWidgets_[ParameterType::FILTER_TYPE] = w ;
    connect(w, &QComboBox::currentTextChanged, this, &ComponentDetailWidget::onValueChange);
}

void ComponentDetailWidget::createSpinWidget(ParameterType p){
    auto w = new QDoubleSpinBox(this);
    
    auto step = GET_PARAMETER_TRAIT_MEMBER(p, uiStepPrecision);
    w->setRange(GET_PARAMETER_TRAIT_MEMBER(p, minimum), GET_PARAMETER_TRAIT_MEMBER(p, maximum));
    w->setSingleStep(step);
    w->setValue(GET_PARAMETER_TRAIT_MEMBER(p, defaultValue));

    parameterWidgets_[p] = w ;
    connect(w, &QDoubleSpinBox::valueChanged, this, &ComponentDetailWidget::onValueChange);
}

void ComponentDetailWidget::setupLayout(){
    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    QHBoxLayout* buttonLayout = new QHBoxLayout();
    QHBoxLayout* parameterLayout = new QHBoxLayout();


    // sequence ( if applicable)
    if ( descriptor_.sequenceable ){
        auto* scroll = new QScrollArea() ;
        QHBoxLayout* sequenceLayout = new QHBoxLayout();
        sequenceWidget_ = new SequenceWidget(componentId_);
        scroll->setWidget(sequenceWidget_);
        sequenceLayout->addWidget(scroll);
        mainLayout->addLayout(sequenceLayout);
    }

    // parameters
    parameterLayout->setSpacing(PARAMETER_WIDGET_SPACING);

    for ( auto p : descriptor_.controllableParameters ){
        QWidget* w = parameterWidgets_[p];
        QHBoxLayout* row = new QHBoxLayout();
        std::string name = GET_PARAMETER_TRAIT_MEMBER(p, name);
        QString labelText = QString::fromStdString(name);
        QLabel* label = new QLabel(labelText, this);

        row->addWidget(label);
        row->addWidget(w);
        parameterLayout->addLayout(row);
    }

    mainLayout->addLayout(parameterLayout);
    
    // buttons
    closeButton_ = new QPushButton("Close",this);
    resetButton_ = new QPushButton("Reset", this);
    buttonLayout->addWidget(closeButton_);
    buttonLayout->addWidget(resetButton_);
    mainLayout->addLayout(buttonLayout);

    // resize this widget to fit all parameters
    int nParameters = parameterWidgets_.size();
    int width = (nParameters * PARAMETER_WIDGET_WIDTH) + ((nParameters - 1) * PARAMETER_WIDGET_SPACING) + MODULE_DETAIL_MARGINS ;
    resize(width, sizeHint().height());
}

void ComponentDetailWidget::closeEvent(QCloseEvent* event){
    event->ignore();
    hide();
    emit widgetClosed();
}

void ComponentDetailWidget::onCloseButtonClicked(){
    hide();
    emit widgetClosed();
}

void ComponentDetailWidget::onValueChange(){
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

    pendingChanges_[p] = v ;

    emit wasModified();
    parameterChangedTimer_->start();
}

void ComponentDetailWidget::flushPendingChanges(){
    if ( pendingChanges_.empty() ) return ;

    for ( auto [p, val] : pendingChanges_ ){
        QJsonObject obj ;
        obj["action"] = "set_component_parameter" ;
        obj["componentId"] = componentId_ ;
        obj["parameter"] = static_cast<int>(p);
        obj["value"] = QVariant::fromStdVariant(val).toJsonValue();
        
        ApiClient::instance()->sendMessage(obj); 
    }

    pendingChanges_.clear();
}