/*
 * Copyright (C) 2026 Jared Burton
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

#include "views/GroupEditor.hpp"
#include "models/ComponentModel.hpp"
#include "app/Theme.hpp"

#include <QScrollArea>

GroupEditor::GroupEditor(QWidget* parent):
    QWidget(parent),
    params_(),
    paramsLayout_(new QGridLayout()),
    closeButton_(new QPushButton("Close",this))
{
    setWindowFlags(Qt::Tool | Qt::WindowStaysOnTopHint);
    setAttribute(Qt::WA_ShowWithoutActivating);

    setupLayout();
}

void GroupEditor::addComponent(ComponentModel* model){
    if ( !model ) return ;
    int id = model->getId();

    if ( params_.contains(id) ) return ;
    int count = params_.size() ;

    params_[id] = new ComponentParameters(model, this);

    // handle layout
    int row = count / Theme::GROUP_EDITOR_GRID_MAX_COLUMNS ;
    int col = count % Theme::GROUP_EDITOR_GRID_MAX_COLUMNS ;

    paramsLayout_->addWidget(params_[id], row, col);

    connect(
        params_[id], &ComponentParameters::parameterEdited,
        this, &GroupEditor::parameterEdited
    );
}

void GroupEditor::removeComponent(ComponentModel* model){
    if ( !model ) return ;
    int id = model->getId();

    if ( !params_.contains(id) ) return ;
    
    paramsLayout_->removeWidget(params_.at(id));
    params_.at(id)->deleteLater();
    params_.erase(id);
    
    // handle grid layout (fill in gap of removed component)
    relayoutParams();
}

ComponentParameters* GroupEditor::getComponentParameters(int componentId){
    if ( !params_.contains(componentId) ) return nullptr ;
    return params_.at(componentId);
}

void GroupEditor::setupLayout(){
    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(
        Theme::COMPONENT_DETAIL_MARGINS,
        Theme::COMPONENT_DETAIL_MARGINS,
        Theme::COMPONENT_DETAIL_MARGINS,
        Theme::COMPONENT_DETAIL_MARGINS
    );

    // parameters
    auto gridContainer = new QWidget();
    gridContainer->setLayout(paramsLayout_);
    auto scroll = new QScrollArea(this);
    scroll->setWidget(gridContainer);
    scroll->setWidgetResizable(true);
    scroll->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    scroll->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    
    mainLayout->addWidget(scroll, 1);

    // buttons
    QHBoxLayout* buttonLayout = new QHBoxLayout();
    buttonLayout->addWidget(closeButton_);
    mainLayout->addLayout(buttonLayout);

    connect(
        closeButton_, &QPushButton::clicked, 
        this, &GroupEditor::onCloseButtonClicked 
    );

}

void GroupEditor::relayoutParams(){
    for ( auto [id, p]: params_ ){
        paramsLayout_->removeWidget(p);
    }

    int count = 0 ;
    for ( auto [id, p]: params_ ){
        int row = count / Theme::GROUP_EDITOR_GRID_MAX_COLUMNS ;
        int col = count % Theme::GROUP_EDITOR_GRID_MAX_COLUMNS ;
        paramsLayout_->addWidget(p, row, col);
        count++ ;
    }

    adjustSize();
}

void GroupEditor::onCloseButtonClicked(){
    hide();
}
