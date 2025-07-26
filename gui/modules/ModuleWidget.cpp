#include "modules/ModuleWidget.hpp"
#include "meta/ModuleRegistry.hpp"

ModuleWidget::ModuleWidget(int id, ModuleType typ, QGraphicsItem* parent): 
    QGraphicsObject(parent),
    moduleId_(id),
    type_(typ),
    width_(100),
    height_(150)
{
    setFlag(ItemIsMovable);
    setFlag(ItemIsSelectable);
    setAcceptedMouseButtons(Qt::LeftButton);
}

QRectF ModuleWidget::boundingRect() const {
    return QRectF(0, 0, width_, height_);
}

void ModuleWidget::paint(QPainter* painter, const QStyleOptionGraphicsItem*, QWidget*) {
    QString name = QString::fromStdString(ModuleRegistry::getModuleDescriptor(type_).name);

    painter->setBrush(Qt::gray);
    painter->drawRect(boundingRect());

    painter->setPen(Qt::white);
    painter->drawText(boundingRect(), Qt::AlignCenter, name);
}