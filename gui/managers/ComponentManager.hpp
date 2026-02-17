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
#include "types/ComponentType.hpp"
#include "requests/CollectionRequest.hpp"

#include <QObject>

class ComponentManager : public QObject {
    Q_OBJECT

private:
    std::map<int, ComponentModel*> models_ ;
    std::map<int, ComponentEditor*> editors_ ;

public:
    ComponentManager(QObject* parent = nullptr);

    // API requests
    void requestAddComponent(ComponentType type);
    void requestRemoveComponent(int componentId);
    void requestParameterUpdate(int componentId, ParameterType p, ParameterValue v);
    void requestCollectionUpdate(CollectionRequest req);

    ComponentModel* getModel(int componentId) const ;
    ComponentEditor* getEditor(int componentId) const ;
    
    void showEditor(int componentId);
    
private:
    // on api response
    void addComponent(int componentId, ComponentType type);
    void removeComponent(int componentId);

    bool handleCollectionApiResponse(const QJsonObject &json);

public slots:
    void onApiDataReceived(const QJsonObject &json);
    void onParameterEdited(int componentId, ParameterType p, ParameterValue value);
    void onCollectionEdited(CollectionRequest req );

signals:
    void componentAdded(int componentId, ComponentType type);
    void componentRemoved(int componentId);
};

#endif // COMPONENT_MANAGER_HPP_