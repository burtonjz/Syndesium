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

#ifndef __UI_GRAPH_PANEL_HPP_
#define __UI_GRAPH_PANEL_HPP_

#include <QGraphicsView>
#include <QGraphicsScene>
#include <QPointer>
#include <vector>

#include "meta/ComponentDescriptor.hpp"
#include "patch/ConnectionManager.hpp"
#include "widgets/SocketContainerWidget.hpp"
#include "widgets/ModuleDetailWidget.hpp"

class ComponentWidget ; // forward declaration

class GraphPanel : public QGraphicsView {
    Q_OBJECT

private:
    QGraphicsScene* scene_ ;
    ConnectionManager* connectionManager_ ;

    // logic for managing socket hovers
    bool isDraggingConnection_ = false ;
    QPointer<SocketWidget> lastHovered_ = nullptr ;

    std::vector<SocketContainerWidget*> widgets_ ;
    std::vector<ModuleDetailWidget*> details_ ;

    // context menu actions
    SocketWidget* clickedSocket_ ;
    QAction* disconnectAllAct_ ;

public:
    explicit GraphPanel(QWidget* parent = nullptr);
    ~GraphPanel();

    void addComponent(int id, ComponentType type);
    void addAudioOutput();
    void addMidiInput();
    
    void deleteSelectedModules();

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
    void componentDoubleClicked(ComponentWidget* widget);

    // size & scale
    static constexpr int DOUBLE_CLICK_MS = 300 ;
    static constexpr qreal GRID_SIZE = 20.0 ;
    static constexpr qreal WHEEL_SCALE_FACTOR = 1.15 ;

private slots:
    void onApiDataReceived(const QJsonObject &json);
    
public  slots:
    void onComponentAdded(ComponentType type);
    void onWidgetZUpdate();

};

#endif // __UI_GRAPH_PANEL_HPP_