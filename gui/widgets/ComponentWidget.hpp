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

#ifndef __UI_COMPONENT_WIDGET_HPP_
#define __UI_COMPONENT_WIDGET_HPP_

#include <QGraphicsObject>
#include <QPainter>
#include <QString>
#include <QStyleOptionGraphicsItem>
#include <qnamespace.h>

#include "widgets/SocketContainerWidget.hpp"
#include "meta/ComponentDescriptor.hpp"
#include "patch/SocketWidget.hpp"

class ComponentWidget :  public SocketContainerWidget {
    Q_OBJECT

private:
    int componentId_ ;
    ComponentDescriptor descriptor_ ;
    
public:
    explicit ComponentWidget(int id, ComponentType type, QGraphicsItem* parent = nullptr);
    ~ComponentWidget() = default ;

    const ComponentDescriptor& getComponentDescriptor() const { return descriptor_ ; }
    const int getID() const { return componentId_ ; }
};

#endif // __UI_COMPONENT_WIDGET_HPP_