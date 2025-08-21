#ifndef __UI_MODULE_WIDGET_HPP_
#define __UI_MODULE_WIDGET_HPP_

#include <QGraphicsObject>
#include <QPainter>
#include <QString>
#include <QStyleOptionGraphicsItem>
#include <qnamespace.h>

#include "widgets/SocketContainerWidget.hpp"
#include "meta/ModuleDescriptor.hpp"
#include "patch/SocketWidget.hpp"

class ModuleWidget :  public SocketContainerWidget {
    Q_OBJECT

private:
    int moduleId_ ;
    ModuleDescriptor descriptor_ ;
    
public:
    explicit ModuleWidget(int id, ModuleType type, QGraphicsItem* parent = nullptr);
    const ModuleDescriptor& getModuleDescriptor() const { return descriptor_ ; }
    const int getID() const { return moduleId_ ; }
};

#endif // __UI_MODULE_WIDGET_HPP_