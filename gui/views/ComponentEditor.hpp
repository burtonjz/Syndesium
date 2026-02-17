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

#ifndef COMPONENT_EDITOR_HPP_
#define COMPONENT_EDITOR_HPP_

#include "models/ComponentModel.hpp"
#include "types/ParameterType.hpp"
#include "widgets/ParameterWidget.hpp"

#include <QWidget>
#include <QPushButton>
#include <QTimer>
#include <unordered_map>

class ComponentEditor : public QWidget {
    Q_OBJECT
    
private:
    ComponentModel* model_ ;
    QPushButton* resetButton_ ;
    QPushButton* closeButton_ ;
    std::map<ParameterType, ParameterWidget*> parameterWidgets_ ; // general independent parameters
    QWidget* componentWidget_ ; // optional component-wide widget for more feature-rich UI

    QTimer* parameterChangedTimer_ ;
    std::unordered_map<ParameterType, ParameterValue> pendingChanges_ ;

public:
    explicit ComponentEditor(ComponentModel* model, QWidget* parent = nullptr);
    ~ComponentEditor() override = default ;

    ComponentModel* getModel() const ;
    
protected:
    ParameterWidget* createParameterWidget(ParameterType p);
    QWidget* createComponentWidget(ComponentType t);

private:
    void setupLayout();
    void closeEvent(QCloseEvent* event) override ;

    void requestCollectionItemCreate(int index, std::function<void(bool)> callback);
    void requestCollectionItemDelete(int index, std::function<void(bool)> callback);

signals:
    void widgetClosed();
    void parameterEdited(int componentId, ParameterType p, ParameterValue value);
    void wasModified();

private slots:
    void onCloseButtonClicked();
    void onResetButtonClicked();

    void flushPendingChanges();

public slots:
    // pass along parameter updates from child ParameterWidgets
    void onValueChange(); 
};


#endif // COMPONENT_EDITOR_HPP_