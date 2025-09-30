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

#ifndef __UI_SOCKET_CONTAINER_WIDGET_HPP_
#define __UI_SOCKET_CONTAINER_WIDGET_HPP_

#include <QGraphicsObject>
#include <QPainter>
#include <QString>
#include <QStyleOptionGraphicsItem>
#include <initializer_list>
#include <vector>

#include "patch/SocketWidget.hpp"

class SocketContainerWidget :  public QGraphicsObject {
    Q_OBJECT

private:
    bool isDragging_ = false ;
    QPointF dragStartPos_ ;
    QString name_ ;

protected:
    QList<SocketWidget*> sockets_ ; 
    QGraphicsTextItem* titleText_ ;
    
    static constexpr qreal COMPONENT_WIDTH = 120.0 ;
    static constexpr qreal COMPONENT_HEIGHT = 80.0 ;
    static constexpr qreal COMPONENT_ROUNDED_RADIUS = 5.0 ;
    static constexpr qreal COMPONENT_BORDER_WIDTH = 2.0 ;
    static constexpr qreal COMPONENT_TEXT_PADDING = 5.0 ;
    static constexpr qreal HIGHLIGHT_BUFFER = 2.0 ; 
    static constexpr qreal HIGHLIGHT_WIDTH = 3.0 ;
    
    static constexpr QColor COMPONENT_TEXT_COLOR = QColor(250,250,250);
    static constexpr QColor COMPONENT_BACKGROUND_COLOR = QColor(60,60,60);
    
    static constexpr qreal SOCKET_SPACING = 15.0 ;

public:
    explicit SocketContainerWidget(QString name, QGraphicsItem* parent = nullptr);

    QRectF boundingRect() const override ;
    void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget = nullptr ) override ;

    const QList<SocketWidget*>& getSockets() const { return sockets_ ; }
    const QString& getName() const { return name_ ; }

    void createSockets(std::initializer_list<SocketSpec> specs );
    void createSockets(const std::vector<SocketSpec> specs );

protected:
    // Graphics overrides
    void mousePressEvent(QGraphicsSceneMouseEvent *event) override ;
    void mouseMoveEvent(QGraphicsSceneMouseEvent *event) override ;
    void mouseReleaseEvent(QGraphicsSceneMouseEvent *event) override ;
    QVariant itemChange(GraphicsItemChange change, const QVariant& value ) override ; // for tracking module position changes

    void layoutSockets();

signals:
    void positionChanged();


};

#endif // __UI_SOCKET_CONTAINER_WIDGET_HPP_