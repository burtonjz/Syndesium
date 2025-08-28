#ifndef __UI_MODULE_WIDGET_HPP_
#define __UI_MODULE_WIDGET_HPP_

#include <QGraphicsObject>
#include <QPainter>
#include <QString>
#include <QStyleOptionGraphicsItem>
#include <qnamespace.h>

#include "widgets/SocketContainerWidget.hpp"
#include "meta/ComponentDescriptor.hpp"
#include "patch/SocketWidget.hpp"

class ModuleWidget :  public SocketContainerWidget {
    Q_OBJECT

private:
    int moduleId_ ;
    ComponentDescriptor descriptor_ ;
    
public:
    explicit ModuleWidget(int id, ModuleType type, QGraphicsItem* parent = nullptr);
    const ComponentDescriptor& getComponentDescriptor() const { return descriptor_ ; }
    const int getID() const { return moduleId_ ; }
};

#endif // __UI_MODULE_WIDGET_HPP_