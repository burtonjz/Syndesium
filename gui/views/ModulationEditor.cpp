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

// std::map<std::pair<int, ParameterType>, ModulationControl*> modulationControls_ ;
// std::vector<std::pair<int, ParameterType>> controlOrder_ ;

// QLabel* editorLabel_ ;
// QGridLayout* ctrlLayout_ ;

#include "views/ModulationEditor.hpp"
#include "app/Theme.hpp"

#include <QVBoxLayout>
#include <QScrollArea>
#include <QEvent>
#include <QApplication>

ModulationEditor::ModulationEditor(QString name, QWidget* parent):
    modulationControls_(),
    controlOrder_(),
    editorLabel_(new QLabel(name, this)),
    ctrlLayout_(new QGridLayout()),
    closeButton_(new QPushButton("Close", this))
{
    setWindowTitle(" ");
    QVBoxLayout* mainLayout = new QVBoxLayout(this);

    editorLabel_->setStyleSheet(Theme::getLabelTitleStyle());
    mainLayout->addWidget(editorLabel_, 0, Qt::AlignCenter);
    mainLayout->addStretch();
    QWidget* gridContainer = new QWidget();
    gridContainer->setLayout(ctrlLayout_);
    auto scroll = new QScrollArea(this);
    scroll->setWidget(gridContainer);
    scroll->setWidgetResizable(true);
    scroll->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    scroll->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    mainLayout->addWidget(scroll, 1);
    mainLayout->addWidget(closeButton_);

    connect(
        closeButton_, &QPushButton::clicked, 
        this, &ModulationEditor::onCloseButtonClicked 
    );
}

ModulationEditor::~ModulationEditor(){
    for ( auto [k, v] : modulationControls_ ){
        v->deleteLater();
    }
}

void ModulationEditor::add(ModulationModel* model){
    if ( !model ) return ;
    
    auto key = std::make_pair(model->getId(),model->getType());
    if ( modulationControls_.contains(key) ){
        qWarning() << "modulation control for component " << key.first 
            << " parameter " << GET_PARAMETER_TRAIT_MEMBER(key.second, name)
            << " is already present in editor." ;
        return ;
    }

    auto ctrl = new ModulationControl(key.first, key.second, this);

    connect(
        ctrl, &ModulationControl::modulationDepthEdited,
        this, &ModulationEditor::modulationDepthEdited
    );

    connect(
        ctrl, &ModulationControl::modulationStrategyEdited,
        this, &ModulationEditor::modulationStrategyEdited
    );

    modulationControls_[key] = ctrl ;
    controlOrder_.push_back(key);    
    updateLayout();
}

void ModulationEditor::remove(int componentId, ParameterType p){
    auto key = std::make_pair(componentId,p);

    if ( !modulationControls_.contains(key) ){
        qWarning() << "modulation control for component " << key.first 
            << " parameter " << GET_PARAMETER_TRAIT_MEMBER(key.second, name)
            << " is not present in editor and cannot be removed" ;
        return ;
    }
    auto ctrl = modulationControls_.at(key);

    modulationControls_.erase(key);
    controlOrder_.erase(std::remove(
        controlOrder_.begin(), controlOrder_.end(), key), 
        controlOrder_.end()
    );
    ctrlLayout_->removeWidget(ctrl);
    ctrl->deleteLater();
    updateLayout();
}

void ModulationEditor::setName(const QString& name){
    editorLabel_->setText(name);
}

void ModulationEditor::setModulationStatus(int componentId, ParameterType p, bool active){
    auto key = std::make_pair(componentId, p);

    if ( !modulationControls_.contains(key) ){
        qWarning() << "modulation control for component " << key.first 
            << " parameter " << GET_PARAMETER_TRAIT_MEMBER(key.second, name)
            << " is not present in editor and cannot be removed" ;
        return ;
    }
    auto ctrl = modulationControls_.at(key);
    ctrl->setConnectionStatus(active);
}

void ModulationEditor::changeEvent(QEvent *event){
    // handle close events on focus loss
    if ( event->type() == QEvent::ActivationChange && !isActiveWindow() ){
        QWidget* active = QApplication::activeWindow();
        if ( !active ){
            return ; // ignore null new active windows -- means its a drag/resize/whatever
        }
        onCloseButtonClicked(); 
    }
    QWidget::changeEvent(event); 
}

void ModulationEditor::updateLayout(){
    for ( const auto& [key, v] : modulationControls_ ){
        ctrlLayout_->removeWidget(v);
    }

    int count = 0 ;
    for ( const auto& key : controlOrder_ ){
        int row = count / 2 ;
        int col = count % 2 ;
        ctrlLayout_->addWidget(modulationControls_.at(key), row, col);
        count++ ;
    }
}

void ModulationEditor::closeEvent(QCloseEvent* event){

}

void ModulationEditor::onCloseButtonClicked(){
    hide();
}
