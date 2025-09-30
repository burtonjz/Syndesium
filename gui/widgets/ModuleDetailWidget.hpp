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

#ifndef __UI_MODULE_DETAIL_WIDGET_HPP_
#define __UI_MODULE_DETAIL_WIDGET_HPP_

#include <QWidget>
#include <QLabel>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QPushButton>
#include <QSpinBox>
#include <QComboBox>
#include <QMap>
#include <QEvent>

#include "types/ParameterType.hpp"
#include "meta/ComponentDescriptor.hpp"

class ModuleDetailWidget : public QWidget {
    Q_OBJECT
    
private:
    int componentId_ ;
    ComponentDescriptor descriptor_ ;

    QPushButton* resetButton_ ;
    QPushButton* closeButton_ ;
    QMap<ParameterType, QWidget*> parameterWidgets_ ;

    static constexpr qreal PARAMETER_WIDGET_SPACING = 10 ;
    static constexpr qreal PARAMETER_WIDGET_WIDTH = 120 ;
    static constexpr qreal MODULE_DETAIL_MARGINS = 20 ;

public:
    explicit ModuleDetailWidget(int id, ComponentType typ, QWidget* parent = nullptr);
    ~ModuleDetailWidget() override = default ;

    int getID() const ;
    ComponentType getType() const ;
    
protected:
    void createParameter(ParameterType p);

private:
    void createWaveformWidget();
    void createSpinWidget(ParameterType p);
    void setupLayout();
    void closeEvent(QCloseEvent* event) override ;

signals:
    void widgetClosed();

private slots:
    void onCloseButtonClicked();

};


#endif // __UI_MODULE_DETAIL_WIDGET_HPP_