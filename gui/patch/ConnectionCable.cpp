#include "patch/ConnectionCable.hpp"
#include "patch/SocketWidget.hpp"

#include <QPainter>
#include <QGraphicsScene>
#include <QGraphicsSceneMouseEvent>
#include <cmath>

ConnectionCable::ConnectionCable(SocketWidget* fromSocket, SocketWidget* toSocket, QGraphicsItem* parent): 
    QGraphicsPathItem(parent), 
    fromSocket_(fromSocket), 
    toSocket_(toSocket)
{
    setZValue(5); // Below sockets but above container widget
    setPen(QPen(getCableColor(), 3, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
    updatePath();
}

void ConnectionCable::setFromSocket(SocketWidget* socket){
    fromSocket_ = socket;
    updatePath();
}

void ConnectionCable::setToSocket(SocketWidget* socket){
    toSocket_ = socket;
    updatePath();
}

void ConnectionCable::setEndpoint(const QPointF& point){
    endpoint_  = point;
    updatePath();
}

bool ConnectionCable::isCompatible(SocketWidget* socket) const {
    if (!fromSocket_ || !socket) return false ;
    
    // Can't connect socket to itself
    if (fromSocket_ == socket) return false ;
    
    // Can't connect sockets from the same container widget
    if (fromSocket_->getParent() == socket->getParent()) return false ;
    
    // Must be compatible types
    SocketType fromType = fromSocket_->getType();
    SocketType toType = socket->getType();

    switch ( fromType ){
    case SocketType::MidiInput:
        return toType == SocketType::MidiOutput ;
    case SocketType::MidiOutput:
        return toType == SocketType::MidiInput ;
    case SocketType::ModulationInput:
        return toType == SocketType::SignalOutput || toType == SocketType::ModulationOutput ;
    case SocketType::ModulationOutput:
        return toType == SocketType::SignalInput || toType == SocketType::ModulationInput ;
    case SocketType::SignalInput:
        return toType == SocketType::SignalOutput ;
    case SocketType::SignalOutput:
        return toType == SocketType::SignalInput || toType == SocketType::ModulationInput ;
    default:
        return false ;
    }
}

bool ConnectionCable::involvesWidget(SocketContainerWidget* widget) const {
    return ( fromSocket_ && fromSocket_->getParent() == widget ) ||
           ( toSocket_   &&   toSocket_->getParent() == widget ) ;
}

void ConnectionCable::updatePath(){
    if (!fromSocket_) return ;
    
    QPointF startPoint = fromSocket_->getConnectionPoint();
    QPointF endPoint ;
    
    if ( toSocket_ ) {
        endPoint = toSocket_->getConnectionPoint();
    } else {
        endPoint = endpoint_ ;
    }
    
    QPainterPath newPath = createBezierPath(startPoint, endPoint);
    setPath(newPath);
    
    // Update pen color
    setPen(QPen(getCableColor(), 3, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
}

QPainterPath ConnectionCable::createBezierPath(const QPointF& start, const QPointF& end){
    QPainterPath path ;
    path.moveTo(start) ;
    
    // Calculate control points for a nice curved cable
    qreal dx = end.x() - start.x();
    qreal dy = end.y() - start.y();
    
    // Make curves more pronounced for longer distances
    qreal controlOffset = std::max(50.0, std::abs(dx) * 0.5);
    
    QPointF control1, control2 ;
    
    if ( dx >= 0 ){
        control1 = start + QPointF(controlOffset, 0);
        control2 = end - QPointF(controlOffset, 0);
    } else {
        control1 = start - QPointF(controlOffset, 0);
        control2 = end + QPointF(controlOffset, 0);
    }

    path.cubicTo(control1, control2, end);
    return path;
}

QColor ConnectionCable::getCableColor() const
{
    if (!fromSocket_) return Qt::gray;
    
    switch (fromSocket_->getType()) {
        case SocketType::ModulationInput:
        case SocketType::ModulationOutput:
            return QColor(255, 80, 80); // red
        case SocketType::SignalInput:
        case SocketType::SignalOutput:
            return QColor(80, 255, 80); // green
        case SocketType::MidiInput:
        case SocketType::MidiOutput:
            return QColor(80, 80, 255); // blue
    }
    return Qt::gray;
}

void ConnectionCable::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget){
    painter->setRenderHint(QPainter::Antialiasing);
    
    // Draw a subtle shadow first
    QPen shadowPen = pen();
    shadowPen.setColor(QColor(0, 0, 0, 50));
    shadowPen.setWidth(pen().width() + 2);
    painter->setPen(shadowPen);
    painter->drawPath(path().translated(1, 1));
    
    // Draw the main cable
    QGraphicsPathItem::paint(painter, option, widget);
}