#ifndef __GUI_CONNECTION_MANAGER_HPP_
#define __GUI_CONNECTION_MANAGER_HPP_

#include <QObject>
#include <QGraphicsScene>

#include "patch/ConnectionCable.hpp"
#include "patch/SocketWidget.hpp"

class ConnectionManager: public QObject {
    Q_OBJECT
private:
    QGraphicsScene* scene_ ;
    QList<ConnectionCable*> connections_ ;

    ConnectionCable* dragConnection_ ;
    SocketWidget* dragFromSocket_ ;
public:
    explicit ConnectionManager(QGraphicsScene* scene, QObject* parent = nullptr);

    void startConnection(SocketWidget* fromSocket);
    void updateDragConnection(const QPointF& scenePos);
    void finishConnection(const QPointF& scenePos);
    void cancelConnection();

    bool hasConnection(SocketWidget* socket) const ;
    void removeConnection(SocketWidget* socket);
    void removeAllConnections(ModuleWidget* module);

    const QList<ConnectionCable*>& getConnections() const { return connections_ ; }

private:
    SocketWidget* findSocketAt(const QPointF& scenePos) const ;
    bool canConnect(SocketWidget* from, SocketWidget* to) const ;

private slots:
    void onModulePositionChanged();

};

#endif // __GUI_CONNECTION_MANAGER_HPP_