#ifndef __UI_GRAPH_PANEL_HPP_
#define __UI_GRAPH_PANEL_HPP_

#include <QGraphicsView>
#include <QGraphicsScene>
#include <vector>

#include "patch/SocketWidget.hpp"
#include "types/ModuleType.hpp"
#include "patch/ConnectionManager.hpp"


class ModuleWidget ; // forward declaration

class GraphPanel : public QGraphicsView {
    Q_OBJECT

private:
    QGraphicsScene* scene_ ;
    ConnectionManager* connectionManager_ ;

    std::vector<SocketContainerWidget*> widgets_ ;

public:
    explicit GraphPanel(QWidget* parent = nullptr);
    ~GraphPanel();

    void addModule(int id, ModuleType type);
    void addAudioOutput();
    void addMidiInput();
    
    void deleteSelectedModules();

protected:
    void keyPressEvent(QKeyEvent* event) override ;
    void mousePressEvent(QMouseEvent* event) override ;
    void wheelEvent(QWheelEvent* event) override ;

private:
    void setupScene() ;
    void connectWidgetSignals(SocketContainerWidget* widget);
    void drawBackground(QPainter* painter, const QRectF& rect) override ;
    
    static constexpr qreal GRID_SIZE = 20.0 ;
    static constexpr qreal WHEEL_SCALE_FACTOR = 1.15 ;
    static constexpr QColor GRAPH_GRID_COLOR = QColor(50,50,50);

private slots:
    void onApiDataReceived(const QJsonObject &json);
    void onWidgetDoubleClicked(SocketContainerWidget* widget);
    void onConnectionStarted(SocketWidget* socket);
    void onConnectionDragging(SocketWidget* socket, QPointF scenePos);
    void onConnectionEnded(SocketWidget* socket, QPointF scenePos);

public  slots:
    void onModuleAdded(ModuleType type);



};

#endif // __UI_GRAPH_PANEL_HPP_