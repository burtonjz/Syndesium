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

#include "managers/ComponentManager.hpp"
#include "meta/ComponentRegistry.hpp"
#include "api/ApiClient.hpp"
#include "util/util.hpp"

ComponentManager::ComponentManager(QObject* parent):
    QObject(parent),
    editors_(),
    models_()
{
    connect(
        ApiClient::instance(), &ApiClient::dataReceived, 
        this, &ComponentManager::onApiDataReceived
    );
}

void ComponentManager::requestAddComponent(ComponentType type){
    QJsonObject obj ;
    auto descriptor = ComponentRegistry::getComponentDescriptor(type);

    obj["action"] = "add_component" ;
    obj["name"] = QString::fromStdString(descriptor.name) ;
    obj["type"] = static_cast<int>(type);
    ApiClient::instance()->sendMessage(obj); 
}

void ComponentManager::requestRemoveComponent(int componentId){    
    QJsonObject obj ;
    obj["action"] = "remove_component" ;
    obj["componentId"] = componentId ;
    ApiClient::instance()->sendMessage(obj);
}

void ComponentManager::requestParameterUpdate(int componentId, ParameterType p, ParameterValue v){
    QJsonObject obj ;
    obj["action"] = "set_parameter" ;
    obj["componentId"] = componentId ;
    obj["parameter"] = static_cast<int>(p);
    obj["value"] = QVariant::fromStdVariant(v).toJsonValue();
    
    ApiClient::instance()->sendMessage(obj); 
}

void ComponentManager::requestCollectionUpdate(CollectionRequest req){
    QJsonObject obj = Util::nlohmannToQJsonObject(req);
    ApiClient::instance()->sendMessage(obj); 
}

ComponentModel* ComponentManager::getModel(int componentId) const {
    auto it = models_.find(componentId);
    if ( it == models_.end() ) return nullptr ;

    return it->second ;
}

ComponentEditor* ComponentManager::getEditor(int componentId) const {
    auto it = editors_.find(componentId);
    if ( it == editors_.end() ) return nullptr ;

    return it->second ;
}


void ComponentManager::showEditor(int componentId){
    auto it = editors_.find(componentId);
    if ( it == editors_.end() ){
        qWarning() << "requested editor for invalid component id:" << componentId ;
        return ;
    }
    it->second->show();
    it->second->raise();
}

void ComponentManager::addComponent(int componentId, ComponentType type){
    models_[componentId] = new ComponentModel(componentId, type);

    auto editor = new ComponentEditor(models_[componentId]);
    editors_[componentId] = editor ;

    // editors must communicate parameter edits
    connect(
        editors_[componentId], &ComponentEditor::parameterEdited,
        this, &ComponentManager::onParameterEdited
    );
    
    auto cw = getEditorCollectionWidget(editor);
    if ( cw ){
        qDebug() << "collection edits connecting!";
        connect(
            cw, &CollectionWidget::collectionEdited,
            this, &ComponentManager::onCollectionEdited
        );
    }

    emit componentAdded(componentId, type);
}

void ComponentManager::removeComponent(int componentId){
    auto it = models_.find(componentId);
    if ( it == models_.end() ){
        qWarning() << "could not find model for component to delete";
        return ;
    }

    models_[componentId]->deleteLater();
    editors_[componentId]->deleteLater();
    models_.erase(componentId);
    editors_.erase(componentId);

    emit componentRemoved(componentId);
}


CollectionWidget* ComponentManager::getEditorCollectionWidget(ComponentEditor* editor) const {
    auto specialized = editor->getSpecializedWidget();
    if ( !specialized ) return nullptr ;

    CollectionWidget* cw = dynamic_cast<CollectionWidget*>(specialized);
    if ( !cw ){
        cw = specialized->findChild<CollectionWidget*>();
    }
    
    return cw ;
}

bool ComponentManager::handleCollectionApiResponse(const QJsonObject &json){
    // note: there is no centralized model for managing Collections. So we will 
    // instead break the pattern here and only do this for specialized widget stored
    // in the component editor. 

    // bool response is to determine if the event should be considered "handled" so
    // that we don't eat up the wrong types of request
    CollectionRequest req ;
    try {
        req = Util::QJsonObjectToNlohmann(json);
    } catch (std::exception& e){
        return false ;
    }

    auto it = editors_.find(req.componentId);
    if ( it == editors_.end() ){
        qWarning() << "Could not find model with Component ID" << req.componentId 
            << ". Will not process collection request" ;
        return true ;
    }
    
    auto cw = getEditorCollectionWidget(it->second);
    if ( cw ){
        cw->updateCollection(req);
    }
    
    return true ;
}


void ComponentManager::onApiDataReceived(const QJsonObject &json){
    QString action = json["action"].toString();
    bool success = json["status"] == "success" ;

    if ( action == "add_component" && success ){
        int id = json["componentId"].toInt();
        ComponentType type = static_cast<ComponentType>(json["type"].toInt());
        addComponent(id, type);
        return ;
    }

    if ( action == "remove_component" && success ){
        int id = json["componentId"].toInt();
        removeComponent(id);
        return ;
    }

    if ( action == "set_component_parameter" && success ){
        int id = json["componentId"].toInt();
        auto it = models_.find(id);
        if ( it == models_.end() ){
            qWarning() << "Could not find model with Component ID" << id 
                << ". Will not process set parameter request" ;
            return ;
        }
        ParameterType p = static_cast<ParameterType>(json["parameter"].toInt());
        
        // dispatch to set parameter value with correct variant
        ParameterValue v ;
        auto j = Util::QJsonObjectToNlohmann(json) ;
        switch(p){
        #define X(name) \
        case ParameterType::name: \
            v = static_cast<GET_PARAMETER_VALUE_TYPE(ParameterType::name)>(j["value"]); \
            break ; 
        PARAMETER_TYPE_LIST
        #undef X     
        default:
            break ;
        }
        it->second->setParameterValue(p, v);
        return ;
    }

    if ( success && handleCollectionApiResponse(json) ){
        return ;
    }
}

void ComponentManager::onParameterEdited(int componentId, ParameterType p, ParameterValue value){
    requestParameterUpdate(componentId, p, value);
}

void ComponentManager::onCollectionEdited(CollectionRequest req ){
    qDebug() << "collection edit request received!";
    requestCollectionUpdate(req);
}