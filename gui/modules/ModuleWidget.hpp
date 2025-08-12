#ifndef __UI_MODULE_WIDGET_HPP_
#define __UI_MODULE_WIDGET_HPP_

#include <QGraphicsObject>
#include <QPainter>
#include <QString>
#include <QStyleOptionGraphicsItem>


#include "meta/ModuleDescriptor.hpp"
#include "patch/SocketWidget.hpp"

class ModuleWidget :  public QGraphicsObject {
    Q_OBJECT

private:
    int moduleId_ ;
    ModuleDescriptor descriptor_ ;
    QList<SocketWidget*> sockets_ ; 

    QGraphicsTextItem* titleText_ ;
    bool isDragging_ = false ;
    QPointF dragStartPos_ ;

    static constexpr qreal MODULE_WIDTH = 120.0 ;
    static constexpr qreal MODULE_HEIGHT = 80.0 ;
    static constexpr qreal SOCKET_SPACING = 15.0 ;
    static constexpr QColor MODULE_BACKGROUND_COLOR = QColor(60,60,60);

public:
    explicit ModuleWidget(int id, ModuleType type, QGraphicsItem* parent = nullptr);

    QRectF boundingRect() const override ;
    void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget = nullptr ) override ;

    const ModuleDescriptor& getModuleDescriptor() const { return descriptor_ ; }
    const QList<SocketWidget*>& getSockets() const { return sockets_ ; }

protected:
    void mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event) override;
    void mousePressEvent(QGraphicsSceneMouseEvent *event) override;
    void mouseMoveEvent(QGraphicsSceneMouseEvent *event) override;
    void mouseReleaseEvent(QGraphicsSceneMouseEvent *event) override;

signals:
    void moduleDoubleClicked(ModuleWidget* module);

private:
    void createSockets();
    void layoutSockets();

};

#endif // __UI_MODULE_WIDGET_HPP_