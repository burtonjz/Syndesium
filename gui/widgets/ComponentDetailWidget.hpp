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

#ifndef __UI_COMPONENT_DETAIL_WIDGET_HPP_
#define __UI_COMPONENT_DETAIL_WIDGET_HPP_

#include <QWidget>
#include <QLabel>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QPushButton>
#include <QDoubleSpinBox>
#include <QComboBox>
#include <QMap>
#include <QEvent>
#include <QTimer>

#include <unordered_map>

#include "types/ParameterType.hpp"
#include "types/CollectionType.hpp"

#include "meta/ComponentDescriptor.hpp"

class ComponentDetailWidget : public QWidget {
    Q_OBJECT
    
private:
    int componentId_ ;
    ComponentDescriptor descriptor_ ;
    QPushButton* resetButton_ ;
    QPushButton* closeButton_ ;
    QMap<ParameterType, QWidget*> parameterWidgets_ ;
    QMap<CollectionType, QWidget*> collectionWidgets_ ;

    QTimer* modifiedTimer_ ;
    QTimer* parameterChangedTimer_ ;

    std::unordered_map<ParameterType, ParameterValue> pendingChanges_ ;

public:
    explicit ComponentDetailWidget(int id, ComponentType typ, QWidget* parent = nullptr);
    ~ComponentDetailWidget() override = default ;

    int getID() const ;
    ComponentType getType() const ;
    
protected:
    QWidget* createParameterWidget(ParameterType p);
    QWidget* createCollectionWidget(CollectionDescriptor cd);

private:
    QWidget* createWaveformWidget();
    QWidget* createFilterTypeWidget();
    QWidget* createStatusWidget();

    QWidget* createSpinWidget(ParameterType p);

    QWidget* createIndependentCollection(CollectionDescriptor cd);
    QWidget* createGroupedCollection(CollectionDescriptor cd);
    QWidget* createSynchronizedCollection(CollectionDescriptor cd);

    void requestCollectionItemCreate(int index, std::function<void(bool)> callback);
    void requestCollectionItemDelete(int index, std::function<void(bool)> callback);
    
    void setupLayout();
    void closeEvent(QCloseEvent* event) override ;

signals:
    void widgetClosed();
    void parameterChanged(int componentId, ComponentDescriptor descriptor, ParameterType p, ParameterValue value);
    void wasModified();

private slots:
    void onCloseButtonClicked();
    void onValueChange();
    void flushPendingChanges();
};


#endif // __UI_COMPONENT_DETAIL_WIDGET_HPP_