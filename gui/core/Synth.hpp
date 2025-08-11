#ifndef __UI_SYNTH_HPP_
#define __UI_SYNTH_HPP_

#include <QMainWindow>
#include <QUiLoader>
#include <QFile>
#include <QWidget>
#include <QVBoxLayout>
#include <QPushButton>
#include <qtmetamacros.h>

#include "core/ModuleContext.hpp"
#include "core/GraphPanel.hpp"
#include "core/Setup.hpp"

#include "types/ModuleType.hpp"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class Synth : public QMainWindow {
    Q_OBJECT

private:
    Ui::MainWindow* ui_ ;
    ModuleContext ctx_ ;
    GraphPanel* graph_ ;
    Setup* setup_ ;

public:
    Synth(ModuleContext ctx, QWidget* parent = nullptr);
    ~Synth();

signals:
    void engineStatusChanged(bool status);
    void moduleAdded(ModuleType typ);

private slots:
    void onApiConnected();
    void onApiDataReceived(const QJsonObject &json);
    void onSetupButtonClicked();
    void onStartStopButtonClicked();
    void onEngineStatusChange(bool status);
    void onModuleAdded(int index);


};

#endif // __UI_SYNTH_HPP_