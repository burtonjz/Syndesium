#ifndef __GUI_CONNECTION_CABLE_HPP_
#define __GUI_CONNECTION_CABLE_HPP_

#include <QGraphicsPathItem>

#include "patch/SocketWidget.hpp"

class ConnectionCable : public QGraphicsPathItem {
private:
    SocketWidget* fromSocket_ ;
    SocketWidget* toSocket_ ;
    QPointF endpoint_ ;

public:
    ConnectionCable(SocketWidget* fromSocket, SocketWidget* toSocket = nullptr, QGraphicsItem* parent = nullptr);

    SocketWidget* getFromSocket() const { return fromSocket_ ; }
    SocketWidget* getToSocket() const { return toSocket_ ; }

    void setFromSocket(SocketWidget* socket);
    void setToSocket(SocketWidget* socket);

    void setEndpoint(const QPointF& end);

    bool isComplete() const { return fromSocket_ && toSocket_ ; }
    bool isCompatible(SocketWidget* socket) const ;

    bool involvesModule(ModuleWidget* module) const ;

    void updatePath();

protected:
    void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget *widget ) override ;

private:
    QPainterPath createBezierPath(const QPointF& start, const QPointF& end);
    QColor getCableColor() const ;

};

#endif // __GUI_CONNECTION_CABLE_HPP_