#ifndef __UI_MODULE_WIDGET_HPP_
#define __UI_MODULE_WIDGET_HPP_

#include <QGraphicsObject>
#include <QPainter>
#include <QString>
#include <QStyleOptionGraphicsItem>
#include <qnamespace.h>


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
    static constexpr qreal MODULE_ROUNDED_RADIUS = 5.0 ;
    static constexpr qreal MODULE_BORDER_WIDTH = 2.0 ;
    static constexpr qreal MODULE_TEXT_PADDING = 5.0 ;
    static constexpr qreal HIGHLIGHT_BUFFER = 2.0 ; 
    static constexpr qreal HIGHLIGHT_WIDTH = 3.0 ;
    
    static constexpr QColor MODULE_TEXT_COLOR = QColor(250,250,250);
    static constexpr QColor MODULE_BACKGROUND_COLOR = QColor(60,60,60);
    
    static constexpr qreal SOCKET_SPACING = 15.0 ;
    
    

public:
    explicit ModuleWidget(int id, ModuleType type, QGraphicsItem* parent = nullptr);

    QRectF boundingRect() const override ;
    void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget = nullptr ) override ;

    const ModuleDescriptor& getModuleDescriptor() const { return descriptor_ ; }
    const QList<SocketWidget*>& getSockets() const { return sockets_ ; }

protected:
    void mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event) override ;
    void mousePressEvent(QGraphicsSceneMouseEvent *event) override ;
    void mouseMoveEvent(QGraphicsSceneMouseEvent *event) override ;
    void mouseReleaseEvent(QGraphicsSceneMouseEvent *event) override ;
    QVariant itemChange(GraphicsItemChange change, const QVariant& value ) override ; // for tracking module position changes

signals:
    void moduleDoubleClicked(ModuleWidget* module);
    void modulePositionChanged();

private:
    void createSockets();
    void layoutSockets();

};

#endif // __UI_MODULE_WIDGET_HPP_