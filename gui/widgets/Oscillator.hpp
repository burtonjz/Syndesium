/*
 * Copyright (C) 2025 Jared Burton
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program. If not, see <https://www.gnu.org/licenses/>.
 */

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
