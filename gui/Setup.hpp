#ifndef UI_HPP
#define UI_HPP

#include <QWidget>
#include <QComboBox>
#include "ModuleContext.hpp"


QT_BEGIN_NAMESPACE
namespace Ui {class AudioMidiSetupWidget;}
QT_END_NAMESPACE

class Setup : public QWidget {
    Q_OBJECT

public:
    Setup(ModuleContext ctx, QWidget *parent = nullptr);
    ~Setup();

    void populateSetupComboBox(QComboBox* box, QJsonValue data);

private:
    Ui::AudioMidiSetupWidget* ui_ ;
    ModuleContext ctx_ ;

private slots:
    void onApiDataReceived(const QJsonObject& json);
    void onSetupSubmit();
    void onSetupCompleted();

signals:
    void setupCompleted();

};

#endif // UI_HPP
