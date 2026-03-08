/*
 * Copyright (C) 2026 Jared Burton
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

#ifndef MODULATION_CONTROL_HPP_
#define MODULATION_CONTROL_HPP_

#include "types/ParameterType.hpp"
#include "widgets/ParameterWidget.hpp"
#include "widgets/ModulationIndicator.hpp"

#include <QWidget>
#include <QLabel>
#include <QComboBox>
#include <QPushButton>

class ModulationControl : public QWidget {
    Q_OBJECT

private:
    int componentId_ ;
    ParameterType parameter_ ;

    QLabel* paramLabel_ ;
    SliderWidget* depthSlider_ ;
    QComboBox* strategySelector_ ;
    ModulationIndicator* modIndicator_ ;
    QPushButton* disconnectBtn_ ;

public:
    ModulationControl(int componentId, ParameterType p, QWidget* parent = nullptr);

private:
    void setupLayout();

public slots:
    void onModelDepthChanged(int componentId, ParameterType p, double depth);
    void onModelStrategyChanged(int componentId, ParameterType p, ModulationStrategy strategy);
    void onModelConnectionUpdated(int componentId, ParameterType p, bool active);

signals:
    void modulationDepthEdited(int componentId, ParameterType p, double depth);
    void modulationStrategyEdited(int componentId, ParameterType p, ModulationStrategy strategy);
    void modulationDisconnected(int componentId, ParameterType p);

};

#endif // MODULATION_CONTROL_HPP_