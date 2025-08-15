#include "patch/SocketWidget.hpp"
#include "widgets/SocketContainerWidget.hpp"

#include <QGraphicsSceneMouseEvent>

SocketWidget::SocketWidget(SocketSpec spec, SocketContainerWidget* parent):
    QGraphicsObject(parent),
    spec_(spec),
    parent_(parent)
{
    setFlag(QGraphicsItem::ItemIsSelectable, false);
    setAcceptHoverEvents(true);
    setAcceptedMouseButtons(Qt::LeftButton);
    setZValue(10); // ensures sockets are on top
}

QRectF SocketWidget::boundingRect() const {
    return QRectF(-SOCKET_RADIUS, -SOCKET_RADIUS, SOCKET_RADIUS * 2, SOCKET_RADIUS * 2);
}

void SocketWidget::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget){
    Q_UNUSED(option)
    Q_UNUSED(widget)

    painter->setRenderHint(QPainter::Antialiasing);
    
    // Draw circle
    QColor socketColor = getSocketColor();
    if ( isHovered_ ){
        socketColor = socketColor.lighter(150);
    }

    painter->setBrush(socketColor);
    painter->setPen(QPen(Qt::black, 2));
    painter->drawEllipse(boundingRect());

    // Draw an indicator for input vs output
    if (isOutput()){
        painter->setBrush(Qt::white);
    } else {
        painter->setBrush(Qt::black);
    }
    painter->drawEllipse(-2,-2,4,4);

}

QColor SocketWidget::getSocketColor() const {
    switch(spec_.type){
        case SocketType::ModulationInput:
        case SocketType::ModulationOutput:
            return QColor(255, 100, 100); // red
        case SocketType::SignalInput:
        case SocketType::SignalOutput:
            return QColor(100,255,100); // green
        case SocketType::MidiInput:
        case SocketType::MidiOutput:
            return QColor(100,100,255); // blue
    }
    return Qt::gray ; // shouldn't happen
}

bool SocketWidget::isOutput() const {
    return spec_.type == SocketType::ModulationOutput || 
           spec_.type == SocketType::SignalOutput ||
           spec_.type == SocketType::MidiOutput
    ;
}

bool SocketWidget::isInput() const {
    return !isOutput() ;
}

QPointF SocketWidget::getConnectionPoint() const {
    return mapToScene(0,0);
}
void SocketWidget::mousePressEvent(QGraphicsSceneMouseEvent *event){
    if ( event->button() == Qt::LeftButton ){
        isDragging_ = true ;
        grabMouse() ;
        emit connectionStarted(this);
        event->accept();
        return ;
    }

    QGraphicsObject::mousePressEvent(event);
}

void SocketWidget::mouseMoveEvent(QGraphicsSceneMouseEvent *event){
    if (isDragging_) {
        emit connectionDragging(this, event->scenePos());
        event->accept();
        return ;
    }

    QGraphicsObject::mouseMoveEvent(event);
}

void SocketWidget::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
    if (event->button() == Qt::LeftButton && isDragging_) {
        isDragging_ = false;
        ungrabMouse() ; 
        emit connectionEnded(this, event->scenePos());
        event->accept();
        return ;
    }

    QGraphicsObject::mouseReleaseEvent(event);
}

void SocketWidget::hoverEnterEvent(QGraphicsSceneHoverEvent *event)
{
    isHovered_ = true;
    update();
    QGraphicsObject::hoverEnterEvent(event);
}

void SocketWidget::hoverLeaveEvent(QGraphicsSceneHoverEvent *event)
{
    isHovered_ = false;
    update();
    QGraphicsObject::hoverLeaveEvent(event);
}
