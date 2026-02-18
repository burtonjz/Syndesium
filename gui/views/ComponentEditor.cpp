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

#include "views/ComponentEditor.hpp"

#include "meta/ComponentDescriptor.hpp"
#include "types/ComponentType.hpp"
#include "types/ParameterType.hpp"
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

ComponentEditor::ComponentEditor(ComponentModel* model, QWidget* parent):
    QWidget(parent),
    model_(model)
{
    auto d = model_->getDescriptor();

    setWindowTitle(QString::fromStdString(d.name));
    setWindowFlags(Qt::Tool | Qt::WindowStaysOnTopHint);
    setAttribute(Qt::WA_ShowWithoutActivating);

    // optional component-specific widget ( for more tailored ui )
    specializedWidget_ = createSpecializedWidget(d.type);

    // create generic parameter widgets 
    for ( auto p: d.controllableParameters ){
        parameterWidgets_[p] = createParameterWidget(p);
    }
    
    setupLayout();

    // handle parameter changes
    parameterChangedTimer_ = new QTimer(this);
    parameterChangedTimer_->setSingleShot(true);
    parameterChangedTimer_->setInterval(300);

    connect(
        parameterChangedTimer_, &QTimer::timeout, 
        this, &ComponentEditor::flushPendingChanges
    ); 
}

ComponentModel* ComponentEditor::getModel() const {
    return model_ ;
}

QWidget* ComponentEditor::getSpecializedWidget() const {
    return specializedWidget_ ;
}

ParameterWidget* ComponentEditor::createParameterWidget(ParameterType p){
    ParameterWidget* w ;
    switch(p){
    case ParameterType::WAVEFORM:
        w = new WaveformWidget(this);
        break ;
    case ParameterType::FILTER_TYPE:
        w = new FilterTypeWidget(this);
        break ;
    case ParameterType::STATUS:
        w = new StatusWidget(this);
        break ;
    case ParameterType::DELAY:
        w = new DelayWidget(this);
        break ;
    default:
        w = new SliderWidget(p, this);
        break ;
    }

    connect(
        w, &ParameterWidget::valueChanged, 
        this, &ComponentEditor::onValueChange
    );
    connect(
        model_, &ComponentModel::parameterValueChanged,
        w, &ParameterWidget::onModelParameterChanged
    );
    
    return w ;
}

QWidget* ComponentEditor::createSpecializedWidget(ComponentType t){
    switch(t){
    case ComponentType::Sequencer:
    {
        auto* scroll = new QScrollArea(this) ;
        PianoRollWidget* pianoRoll = new PianoRollWidget(model_, this);
        scroll->setWidget(pianoRoll);
        connect(
            model_, &ComponentModel::parameterValueChanged, 
            pianoRoll, &PianoRollWidget::onParameterChanged
        );
        return scroll ;
    }
    default:
        return nullptr ;
        
    }
}

void ComponentEditor::setupLayout(){
    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(
        Theme::COMPONENT_DETAIL_MARGINS,
        Theme::COMPONENT_DETAIL_MARGINS,
        Theme::COMPONENT_DETAIL_MARGINS,
        Theme::COMPONENT_DETAIL_MARGINS
    );
    
    // component-specific widget belongs at the top
    if ( specializedWidget_ ){
        mainLayout->addWidget(specializedWidget_);
    }

    // parameter widgets horizontally spaced
    QHBoxLayout* parameterLayout = new QHBoxLayout();
    parameterLayout->setSpacing(Theme::COMPONENT_DETAIL_WIDGET_SPACING);

    for ( auto p : model_->getDescriptor().controllableParameters ){
        parameterLayout->addWidget(parameterWidgets_[p]);
    }

    mainLayout->addLayout(parameterLayout);
    
    // buttons
    QHBoxLayout* buttonLayout = new QHBoxLayout();
    closeButton_ = new QPushButton("Close",this);
    resetButton_ = new QPushButton("Reset", this);
    buttonLayout->addWidget(closeButton_);
    buttonLayout->addWidget(resetButton_);
    mainLayout->addLayout(buttonLayout);

    connect(
        closeButton_, &QPushButton::clicked, 
        this, &ComponentEditor::onCloseButtonClicked 
    );
    connect(
        resetButton_, &QPushButton::clicked, 
        this, &ComponentEditor::onResetButtonClicked
    );

    adjustSize();
}

void ComponentEditor::closeEvent(QCloseEvent* event){
    event->ignore();
    hide();
    emit widgetClosed();
}

void ComponentEditor::onCloseButtonClicked(){
    hide();
    emit widgetClosed();
}

void ComponentEditor::onResetButtonClicked(){
    // TODO ask model to reset all parameters to default
}

void ComponentEditor::onValueChange(){
    auto w = dynamic_cast<ParameterWidget*>(sender());
    if ( !w ) return ;

    ParameterType p = w->getType();
    ParameterValue v = w->getValue();
    
    pendingChanges_[p] = v ;
    parameterChangedTimer_->start();
}

void ComponentEditor::flushPendingChanges(){
    if ( pendingChanges_.empty() ) return ;

    for ( auto [p, val] : pendingChanges_ ){
        emit parameterEdited(model_->getId(),p,val) ;
    }
    pendingChanges_.clear();
}