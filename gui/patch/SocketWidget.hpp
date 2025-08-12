#ifndef __GUI_SOCKET_WIDGET_HPP_
#define __GUI_SOCKET_WIDGET_HPP_


#include <QGraphicsObject>
#include <QWidget>
#include <QPainter>
#include <QStyleOptionGraphicsItem>



class ModuleWidget ; // forward declaration

enum class SocketType {
    ModulationInput,
    ModulationOutput,
    SignalInput,
    SignalOutput,
    MidiInput,
    MidiOutput
};

class SocketWidget : public QGraphicsObject {
    Q_OBJECT

private:
    SocketType sockType_ ;
    QString name_ ;
    ModuleWidget* parentModule_ ;
    bool isHovered_ = false ;
    bool isDragging_ = false ;
    QColor getSocketColor() const ;

    static constexpr qreal SOCKET_RADIUS = 6.0 ;

public:
    SocketWidget(SocketType type, QString name, ModuleWidget* parent = nullptr);

    // QGraphicsItem interface
    QRectF boundingRect() const override ;
    void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) override ;

    // Getters
    SocketType getType() const { return sockType_ ; } 
    ModuleWidget* getParent() const { return parentModule_ ; }
    bool isOutput() const ;
    bool isInput() const ;
    QPointF getConnectionPoint() const ;

protected:
    void mousePressEvent(QGraphicsSceneMouseEvent* event) override ;
    void mouseMoveEvent(QGraphicsSceneMouseEvent* event) override ;
    void mouseReleaseEvent(QGraphicsSceneMouseEvent* event) override ;
    void hoverEnterEvent(QGraphicsSceneHoverEvent* event) override ;
    void hoverLeaveEvent(QGraphicsSceneHoverEvent* event) override ;

signals:
    void connectionStarted(SocketWidget* socket);
    void connectionDragging(SocketWidget* socket, QPointF scenePos);
    void connectionEnded(SocketWidget* socket, QPointF scenePos);

};

#endif // __GUI_SOCKET_WIDGET_HPP_