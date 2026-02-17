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

#ifndef GRAPH_PANEL_HPP_
#define GRAPH_PANEL_HPP_

#include <QGraphicsView>
#include <QGraphicsScene>
#include <QPointer>
#include <QJsonArray>
#include <qjsonobject.h>
#include <vector>

#include "interfaces/ISocketLookup.hpp"
#include "managers/ConnectionManager.hpp"
#include "views/ConnectionRenderer.hpp"
#include "managers/ComponentManager.hpp"
#include "graphics/GraphNode.hpp"

class ComponentNode ; // forward declaration

class GraphPanel : public QGraphicsView, public ISocketLookup {
    Q_OBJECT

private:
    QGraphicsScene* scene_ ;
    ConnectionRenderer* connectionRenderer_ ;
    ConnectionManager* connectionManager_ ;
    ComponentManager* componentManager_ ;
    
    std::vector<GraphNode*> nodes_ ;
    
    // logic for managing socket hovers
    bool isDraggingConnection_ = false ;
    QPointer<SocketWidget> lastHovered_ = nullptr ;

    // context menu actions
    SocketWidget* clickedSocket_ ;
    QAction* disconnectAllAct_ ;

    // hardware widgets
    GraphNode* audioOut_ ;
    GraphNode* midiIn_ ;

public:
    explicit GraphPanel(QWidget* parent = nullptr);
    ~GraphPanel();

    void addAudioOutput();
    void addMidiInput();
    
    QJsonArray getComponentPositions() const ;

    void loadConnection(const QJsonObject& request); 
    void loadPositions(const QJsonObject& request);

    ComponentNode* getNode(int ComponentId) const ;
    SocketWidget* getNodeSocket(
        GraphNode* w, SocketType t, 
        std::variant<std::monostate, size_t, ParameterType> selector = std::monostate{} 
    );

    std::vector<ComponentNode*> getSelectedComponents() const ;

    // ISocketLookup
    SocketWidget* findSocket(
        SocketType type,
        std::optional<int> componentId = std::nullopt,
        std::optional<size_t> idx = std::nullopt, 
        std::optional<ParameterType> param = std::nullopt        
    ) override ;
    SocketWidget* findSocketAt(const QPointF& scenePos) override ;

protected:
    void keyPressEvent(QKeyEvent* event) override ;
    void mousePressEvent(QMouseEvent* event) override ;
    void mouseDoubleClickEvent(QMouseEvent* event) override ;
    void wheelEvent(QWheelEvent* event) override ;
    void mouseMoveEvent(QMouseEvent* event) override ;
    void mouseReleaseEvent(QMouseEvent* event) override ;
    void contextMenuEvent(QContextMenuEvent *event) override;

private:
    void setupScene() ;
    void createContextMenuActions() ;

    void drawBackground(QPainter* painter, const QRectF& rect) override ;
    void componentDoubleClicked(ComponentNode* widget);

private slots:
    void onApiDataReceived(const QJsonObject &json);
    
public  slots:
    // from parent
    void onComponentSelected(ComponentType type);
    
    // from component manager
    void onComponentAdded(int componentId, ComponentType type);
    void onComponentRemoved(int componentId);

    void onNodeZUpdate();

signals:
    void wasModified();

};

#endif // GRAPH_PANEL_HPP_