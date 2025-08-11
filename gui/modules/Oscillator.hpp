#ifndef OSCILLATOR_HPP
#define OSCILLATOR_HPP

#include "core/ApiClient.hpp"
#include <QWidget>
#include <QComboBox>

namespace Ui {
class Oscillator;
}

class Oscillator : public QWidget {
    Q_OBJECT

public:
    explicit Oscillator(ApiClient* apiClient, int moduleId, QWidget *parent = nullptr);
    ~Oscillator();

    void initialize();

private:
    Ui::Oscillator* ui ;
    ApiClient* client ;
    int id ;

    void populateWaveformComboBox(QComboBox* box, QJsonValue data);

private slots:
    void onApiDataReceived(const QJsonObject &json);

    void onWaveformComboBoxChanged(int index);

};

#endif // OSCILLATOR_HPP
