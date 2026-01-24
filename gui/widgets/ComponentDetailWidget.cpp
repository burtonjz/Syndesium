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
#include "types/CollectionType.hpp"
#include "types/ComponentType.hpp"
#include "types/ParameterType.hpp"
#include "types/Waveform.hpp"
#include "types/FilterType.hpp"
#include "core/ApiClient.hpp"
#include "core/Theme.hpp"

#include "widgets/PianoRollWidget.hpp"
#include "widgets/SwitchWidget.hpp"

#include <QJsonObject>
#include <QEvent>
#include <QCloseEvent>
#include <QBoxLayout>
#include <QScrollArea>
#include <qboxlayout.h>
#include <qscrollarea.h>

ComponentDetailWidget::ComponentDetailWidget(int id, ComponentType type, QWidget* parent):
    QWidget(parent),
    componentId_(id),
    descriptor_(ComponentRegistry::getComponentDescriptor(type))
{
    setWindowTitle(QString::fromStdString(descriptor_.name));

    setWindowFlags(Qt::Tool | Qt::WindowStaysOnTopHint);
    setAttribute(Qt::WA_ShowWithoutActivating);

    for ( auto p: descriptor_.controllableParameters ){
        createParameterWidget(p);
    }

    for ( auto cd : descriptor_.collections ){
        createCollectionWidget(cd);
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

void ComponentDetailWidget::createParameterWidget(ParameterType p){
    switch(p){
    case ParameterType::WAVEFORM:
        parameterWidgets_[p] = createWaveformWidget();
        break ;
    case ParameterType::FILTER_TYPE:
        parameterWidgets_[p] = createFilterTypeWidget();
        break ;
    case ParameterType::STATUS:
        parameterWidgets_[p] = createStatusWidget();
        break ;
    default:
        // most things will just be dials
        parameterWidgets_[p] = createSpinWidget(p);
        break ;
    }
}

void ComponentDetailWidget::createCollectionWidget(CollectionDescriptor cd){
    // specialized widgets defined here
    if ( cd.collectionType == CollectionType::SEQUENCER ){
        PianoRollWidget* pianoRoll = new PianoRollWidget(componentId_);
        collectionWidgets_[cd.collectionType] = pianoRoll ;
        connect(this, &ComponentDetailWidget::parameterChanged, pianoRoll, &PianoRollWidget::onParameterChanged);
        return ;
    }

    // TODO: generic collection widget creation
    
}

QWidget* ComponentDetailWidget::createWaveformWidget(){
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

    connect(w, &QComboBox::currentTextChanged, this, &ComponentDetailWidget::onValueChange);
    return w ;
}

QWidget* ComponentDetailWidget::createFilterTypeWidget(){
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

    connect(w, &QComboBox::currentTextChanged, this, &ComponentDetailWidget::onValueChange);
    return w ;
}

QWidget* ComponentDetailWidget::createStatusWidget(){
    auto w = new SwitchWidget(this);

    connect(w, &QAbstractButton::toggled, this, &ComponentDetailWidget::onValueChange);
    return w ;
}

QWidget* ComponentDetailWidget::createSpinWidget(ParameterType p){
    auto w = new QDoubleSpinBox(this);
    
    auto step = GET_PARAMETER_TRAIT_MEMBER(p, uiStepPrecision);
    w->setRange(GET_PARAMETER_TRAIT_MEMBER(p, minimum), GET_PARAMETER_TRAIT_MEMBER(p, maximum));
    w->setSingleStep(step);
    w->setValue(GET_PARAMETER_TRAIT_MEMBER(p, defaultValue));

    connect(w, &QDoubleSpinBox::valueChanged, this, &ComponentDetailWidget::onValueChange);

    return w ;
}

void ComponentDetailWidget::setupLayout(){
    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    QHBoxLayout* buttonLayout = new QHBoxLayout();
    

    // collections
    QVBoxLayout* collectionLayout = new QVBoxLayout();
    for ( auto cd : descriptor_.collections ){
        QWidget* w = collectionWidgets_[cd.collectionType];

        // specialized first
        switch(cd.collectionType){
        case CollectionType::SEQUENCER:
        {
            auto* scroll = new QScrollArea() ;
            scroll->setWidget(w);
            collectionLayout->addWidget(scroll);
            break ;
        }
        case CollectionType::GENERIC:
        default:
            qWarning() << "attempted to make collection widget of type" << CollectionType::toString(cd.collectionType) << ", which is not implemented" ;
            break ;
        }
        if ( cd.collectionType == CollectionType::SEQUENCER ){    
            
        } 

        // TODO: generic collection layout logic
    }

    mainLayout->addLayout(collectionLayout);

    // parameters
    QHBoxLayout* parameterLayout = new QHBoxLayout();
    parameterLayout->setSpacing(Theme::COMPONENT_DETAIL_WIDGET_SPACING);

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
    int width = (nParameters * Theme::COMPONENT_DETAIL_WIDGET_WIDTH) + 
        ((nParameters - 1) * Theme::COMPONENT_DETAIL_WIDGET_SPACING) + 
        Theme::COMPONENT_DETAIL_MARGINS ;
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
    } else if ( auto st = dynamic_cast<SwitchWidget*>(it.value())){
        v = st->isChecked();
    } else {
        return ;
    }

    pendingChanges_[p] = v ;
    parameterChangedTimer_->start();
}

void ComponentDetailWidget::flushPendingChanges(){
    if ( pendingChanges_.empty() ) return ;

    for ( auto [p, val] : pendingChanges_ ){
        QJsonObject obj ;
        obj["action"] = "set_parameter" ;
        obj["componentId"] = componentId_ ;
        obj["parameter"] = static_cast<int>(p);
        obj["value"] = QVariant::fromStdVariant(val).toJsonValue();
        
        ApiClient::instance()->sendMessage(obj); 
        emit parameterChanged(componentId_,descriptor_,p,val) ;
    }

    pendingChanges_.clear();
    emit wasModified();
}