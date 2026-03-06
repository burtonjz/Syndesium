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
    models_(),
    editors_(),
    groupModels_(),
    groupEditors_()
{
    connect(
        ApiClient::instance(), &ApiClient::dataReceived, 
        this, &ComponentManager::onApiDataReceived
    );
}

ComponentManager::~ComponentManager(){
    for ( const auto& m : models_ ) m.second->deleteLater();
    for ( const auto& e : editors_ ) e.second->deleteLater();
    for ( const auto& gm: groupModels_ ) gm.second->deleteLater();
    for ( const auto& ge : groupEditors_ ) ge.second->deleteLater();
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

void ComponentManager::requestModulationDepthUpdate(int componentId, ParameterType p, double depth){
    QJsonObject obj ;
    obj["action"] = "set_modulation_depth" ;
    obj["componentId"] = componentId ;
    obj["parameter"] = QString::fromStdString(GET_PARAMETER_TRAIT_MEMBER(p, name));
    obj["depth"] = depth ;

    ApiClient::instance()->sendMessage(obj);
}

void ComponentManager::requestModulationStrategyUpdate(int componentId, ParameterType p, ModulationStrategy strategy){
    QJsonObject obj ;
    obj["action"] = "set_modulation_strategy" ;
    obj["componentId"] = componentId ;
    obj["parameter"] = QString::fromStdString(GET_PARAMETER_TRAIT_MEMBER(p, name));
    obj["strategy"] = static_cast<int>(strategy) ;

    ApiClient::instance()->sendMessage(obj);
}



void ComponentManager::renameComponent(int id, const QString& name){
    auto editor = getEditor(id);

    if ( !editor ) return ;
    editor->setName(name);
}

void ComponentManager::renameGroup(int id, const QString& name){
    auto editor = getGroupEditor(id);

    if ( !editor ) return ;
    editor->setName(name);
}

ComponentModel* ComponentManager::getModel(int componentId) const {
    if ( !models_.contains(componentId) ) return nullptr ;

    return models_.at(componentId) ;
}

ComponentEditor* ComponentManager::getEditor(int componentId) const {
    if ( !editors_.contains(componentId) ) return nullptr ;

    return editors_.at(componentId) ;
}

GroupModel* ComponentManager::getGroupModel(int groupId) const {
    if ( !groupModels_.contains(groupId) ) return nullptr ;

    return groupModels_.at(groupId) ;
}

GroupEditor* ComponentManager::getGroupEditor(int groupId) const {
    if ( !groupEditors_.contains(groupId) ) return nullptr ;

    return groupEditors_.at(groupId) ;
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

void ComponentManager::showGroupEditor(int groupId){
    auto it = groupEditors_.find(groupId);
    if ( it == groupEditors_.end() ){
        qWarning() << "requested group editor for invalid group id:" << groupId ;
        return ;
    }
    it->second->show();
    it->second->raise();
}

void ComponentManager::createGroup(const std::vector<int> componentIds){
    int id = currentGroupId_++ ;
    GroupModel* m = new GroupModel(id);
    GroupEditor* g = new GroupEditor(QString("Group %1").arg(id));
    groupModels_[id] = m ;
    groupEditors_[id] = g ;

    connect(
        g, &GroupEditor::parameterEdited,
        this, &ComponentManager::onParameterEdited
    );

    for ( auto i : componentIds ){
        auto model = getModel(i);
        m->addComponent(model);
        g->addComponent(model);
        
        // handle specialized collection widgets
        auto cw = getCollectionWidget(g->getComponentParameters(i));
        if ( cw ){
            connect(
                cw, &CollectionWidget::collectionEdited,
                this, &ComponentManager::onCollectionEdited
            );
        }
    }

    emit componentGroupCreated(id, componentIds);
}

void ComponentManager::appendToGroup(int groupId, const std::vector<int> componentIds){
    auto model = getGroupModel(groupId);
    auto editor = getGroupEditor(groupId);
    
    if ( !model || !editor ){
        qWarning() << "Could not find group editor with Id " << groupId ;
        return ;
    }
    
    for ( auto id : componentIds ){
        model->addComponent(getModel(id));
        editor->addComponent(getModel(id));
    }

    emit componentGroupUpdated(groupId, model->getComponents());
}

void ComponentManager::removeGroup(int groupId){
    auto model = getGroupModel(groupId);
    auto editor = getGroupEditor(groupId);

    if ( !model || !editor ) return ;

    emit componentGroupRemoved(groupId, model->getComponents());

    groupModels_.erase(groupId);
    groupEditors_.erase(groupId);
    
    model->deleteLater();
    editor->deleteLater();    
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
    
    auto cw = getCollectionWidget(editor->getComponentParameters());
    if ( cw ){
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


CollectionWidget* ComponentManager::getCollectionWidget(ComponentParameters* params) const {
    if ( !params ) return nullptr ;
    auto specialized = params->getSpecializedWidget();
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
    
    auto cw = getCollectionWidget(it->second->getComponentParameters());
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

    if ( action == "set_modulation_depth" && success ){
        int id = json["componentId"].toInt();
        auto it = models_.find(id);
        if ( it == models_.end() ){
            qWarning() << "Could not find model with Component ID" << id 
                << ". Will not process modulation depth request" ;
            return ;
        }

        ParameterType p = parameterFromString(json["parameter"].toString().toStdString());
        ModulationModel* m = it->second->getModulationModel(p);
        if ( !m ){
            qWarning() << "Could not find Modulation Model for Parameter " 
                << GET_PARAMETER_TRAIT_MEMBER(p, name) 
                << "from Component Model: " << it->second ;
            return ;
        }

        m->setDepth(json["depth"].toDouble());
        return ;
    }

    if ( action == "set_modulation_strategy" && success ){
        int id = json["componentId"].toInt();
        auto it = models_.find(id);
        if ( it == models_.end() ){
            qWarning() << "Could not find model with Component ID" << id 
                << ". Will not process modulation strategy request" ;
            return ;
        }

        ParameterType p = parameterFromString(json["parameter"].toString().toStdString());
        ModulationModel* m = it->second->getModulationModel(p);
        if ( !m ){
            qWarning() << "Could not find Modulation Model for Parameter " 
                << GET_PARAMETER_TRAIT_MEMBER(p, name) 
                << "from Component Model: " << it->second ;
            return ;
        }

        m->setStrategy(static_cast<ModulationStrategy>(json["strategy"].toInt()));
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

void ComponentManager::onModulationDepthEdited(int componentId, ParameterType p, double depth){
    requestModulationDepthUpdate(componentId, p, depth);
}

void ComponentManager::onModulationStrategyEdited(int componentId, ParameterType p, ModulationStrategy strategy){
    requestModulationStrategyUpdate(componentId, p, strategy);
}