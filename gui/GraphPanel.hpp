#ifndef __UI_GRAPH_PANEL_HPP_
#define __UI_GRAPH_PANEL_HPP_

#include <QGraphicsView>
#include <QGraphicsScene>
#include <qjsonobject.h>
#include <qtmetamacros.h>

#include "ApiClient.hpp"
#include "types/ModuleType.hpp"

class GraphPanel : public QGraphicsView {
    Q_OBJECT

private:
    QGraphicsScene* scene_ ;
    ApiClient* apiClient_ ;

public:
    explicit GraphPanel(ApiClient* client, QWidget* parent = nullptr);
    void addModule(int id, ModuleType typ);

private slots:
    void onApiDataReceived(const QJsonObject &json);

public  slots:
    void onModuleAdded(ModuleType typ);

};

#endif // __UI_GRAPH_PANEL_HPP_