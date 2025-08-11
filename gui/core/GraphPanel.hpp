#ifndef __UI_GRAPH_PANEL_HPP_
#define __UI_GRAPH_PANEL_HPP_

#include <QGraphicsView>
#include <QGraphicsScene>
#include <qjsonobject.h>
#include <qtmetamacros.h>

#include "core/ApiClient.hpp"
#include "types/ModuleType.hpp"

class GraphPanel : public QGraphicsView {
    Q_OBJECT

private:
    QGraphicsScene* scene_ ;
    ApiClient* apiClient_ ;

public:
    explicit GraphPanel(ApiClient* client, QWidget* parent = nullptr);
    void addModule(int id, ModuleType type);

private slots:
    void onApiDataReceived(const QJsonObject &json);

public  slots:
    void onModuleAdded(ModuleType type);

};

#endif // __UI_GRAPH_PANEL_HPP_