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
    const ComponentDescriptor& getComponentDescriptor() const { return descriptor_ ; }
    const int getID() const { return componentId_ ; }
};

#endif // __UI_COMPONENT_WIDGET_HPP_