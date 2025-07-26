#ifndef __UI_MODULE_WIDGET_HPP_
#define __UI_MODULE_WIDGET_HPP_

#include <QGraphicsObject>
#include <QPainter>
#include <QString>
#include <qstyleoption.h>
#include <qtmetamacros.h>

#include "types/ModuleType.hpp"

class ModuleWidget :  public QGraphicsObject {
    Q_OBJECT

private:
    int moduleId_ ;
    ModuleType type_ ;
    qreal width_ ;
    qreal height_ ;

public:
    explicit ModuleWidget(int id, ModuleType typ, QGraphicsItem* parent = nullptr);

    QRectF boundingRect() const override ;
    void paint(QPainter* painteer, const QStyleOptionGraphicsItem* option, QWidget* widget = nullptr ) override ;
};

#endif // __UI_MODULE_WIDGET_HPP_