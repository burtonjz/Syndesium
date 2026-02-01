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

#ifndef PARAMETER_WIDGET_HPP_
#define PARAMETER_WIDGET_HPP_

#include "types/ParameterType.hpp"


#include <QWidget>
#include <QLabel>
#include <QComboBox>
#include <QSlider>

// base class
class ParameterWidget : public QWidget {
    Q_OBJECT

public:
    explicit ParameterWidget(QWidget* parent = nullptr):
        QWidget(parent)
    {}

    virtual ~ParameterWidget() = default ;

    virtual ParameterValue getValue() const = 0 ;
    virtual void setValue(const ParameterValue& value) = 0 ;

signals:
    void valueChanged();
};

class DelayWidget : public ParameterWidget {
private:
    QLabel* label_ ;
    QSlider* slider_ ;
    QComboBox* unitCombo_ ;
    QLabel* valueLabel_ ;
    double sampleRate_ ;

public:
    explicit DelayWidget(QWidget* parent = nullptr);

    ParameterValue getValue() const override ;
    void setValue(const ParameterValue& value) override ;
    void setValue(size_t samples);

private:
    void setupUI();
    void connectSignals();
    void updateDisplay();
};



#endif // PARAMETER_WIDGET_HPP_