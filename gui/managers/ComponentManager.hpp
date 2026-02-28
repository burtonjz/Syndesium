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

#ifndef COMPONENT_MANAGER_HPP_
#define COMPONENT_MANAGER_HPP_

#include "models/ComponentModel.hpp"
#include "views/ComponentEditor.hpp"
#include "views/GroupEditor.hpp"
#include "types/ComponentType.hpp"
#include "requests/CollectionRequest.hpp"
#include "widgets/CollectionWidget.hpp"
#include "widgets/ComponentParameters.hpp"

#include <QObject>

class ComponentManager : public QObject {
    Q_OBJECT

private:
    std::map<int, ComponentModel*> models_ ;
    std::map<int, ComponentEditor*> editors_ ;
    std::map<int, GroupEditor*> groupEditors_ ;

    int currentGroupId_ = 0 ;

public:
    ComponentManager(QObject* parent = nullptr);
    ~ComponentManager();

    // API requests
    void requestAddComponent(ComponentType type);
    void requestRemoveComponent(int componentId);
    void requestParameterUpdate(int componentId, ParameterType p, ParameterValue v);
    void requestCollectionUpdate(CollectionRequest req);

    ComponentModel* getModel(int componentId) const ;
    ComponentEditor* getEditor(int componentId) const ;
    GroupEditor* getGroupEditor(int groupId) const ;
    
    void showEditor(int componentId);
    void showGroupEditor(int groupId);

    void createGroup(const std::vector<int> componentIds);
    void appendToGroup(int groupId, const std::vector<int> componentIds);
    void removeGroup(int groupId);
    
private:
    // on api response
    void addComponent(int componentId, ComponentType type);
    void removeComponent(int componentId);

    CollectionWidget* getCollectionWidget(ComponentParameters* params) const ;
    bool handleCollectionApiResponse(const QJsonObject &json);

public slots:
    void onApiDataReceived(const QJsonObject &json);
    void onParameterEdited(int componentId, ParameterType p, ParameterValue value);
    void onCollectionEdited(CollectionRequest req );

signals:
    void componentAdded(int componentId, ComponentType type);
    void componentRemoved(int componentId);
    void componentGroupUpdated(int groupId, const std::vector<int> componentIds);
};

#endif // COMPONENT_MANAGER_HPP_