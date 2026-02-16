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
#include "api/ApiClient.hpp"
#include "app/Theme.hpp"

#include "widgets/ParameterWidget.hpp"
#include "widgets/PianoRollWidget.hpp"

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
        parameterWidgets_[p] = createParameterWidget(p);
    }

    for ( auto cd : descriptor_.collections ){
        collectionWidgets_[cd.collectionType] = createCollectionWidget(cd);
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

QWidget* ComponentDetailWidget::createParameterWidget(ParameterType p){
    switch(p){
    case ParameterType::WAVEFORM:
        return new WaveformWidget(this);
    case ParameterType::FILTER_TYPE:
        return new FilterTypeWidget(this);
    case ParameterType::STATUS:
        return new StatusWidget(this);
    case ParameterType::DELAY:
        return new DelayWidget(this);
    default:
        // use generic slider
        return new SliderWidget(p, this);
    }
}

QWidget* ComponentDetailWidget::createCollectionWidget(CollectionDescriptor cd){
    switch(cd.collectionType){
    case CollectionType::SEQUENCER:
    {
        PianoRollWidget* pianoRoll = new PianoRollWidget(componentId_);
        connect(this, &ComponentDetailWidget::parameterChanged, pianoRoll, &PianoRollWidget::onParameterChanged);
        return pianoRoll ;
    }
    case CollectionType::GENERIC:
    default:
    {
        switch (cd.structure){
        case CollectionStructure::INDEPENDENT:
            return createIndependentCollection(cd);
        case CollectionStructure::GROUPED:
            return createGroupedCollection(cd);
        case CollectionStructure::SYNCHRONIZED:
            return createSynchronizedCollection(cd);
        default:
            return nullptr ;
        }
    }}
}

QWidget* ComponentDetailWidget::createIndependentCollection(CollectionDescriptor cd){
    // TODO
    return nullptr ;
}

QWidget* ComponentDetailWidget::createGroupedCollection(CollectionDescriptor cd){
    // TODO
    return nullptr ;
}

QWidget* ComponentDetailWidget::createSynchronizedCollection(CollectionDescriptor cd){
    // TODO
    return nullptr ;
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

        // TODO: generic collection layout logic
    }

    mainLayout->addLayout(collectionLayout);

    // parameters
    QHBoxLayout* parameterLayout = new QHBoxLayout();
    parameterLayout->setSpacing(Theme::COMPONENT_DETAIL_WIDGET_SPACING);

    for ( auto p : descriptor_.controllableParameters ){
        QWidget* w = parameterWidgets_[p];
        parameterLayout->addWidget(w);
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

    if ( auto pWidget = dynamic_cast<ParameterWidget*>(it.value())){
        v = pWidget->getValue();
    } else {
        qWarning() << "invalid QWidget stored for parameter of type " << GET_PARAMETER_TRAIT_MEMBER(p,name) ;
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