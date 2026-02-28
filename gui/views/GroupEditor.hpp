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

#ifndef GROUP_EDITOR_HPP_
#define GROUP_EDITOR_HPP_

#include "widgets/ComponentParameters.hpp"
#include "types/ParameterType.hpp"

#include <QWidget>
#include <QPushButton>
#include <QGridLayout>
#include <unordered_map>

// forward declarations
class ComponentModel ;

class GroupEditor : public QWidget {
    Q_OBJECT

private:
    std::unordered_map<int, ComponentParameters*> params_ ;
    QGridLayout* paramsLayout_ ;
    QPushButton* closeButton_ ;

public:
    explicit GroupEditor(QWidget* parent = nullptr);

    void addComponent(ComponentModel* model);
    void removeComponent(ComponentModel* model);
    ComponentParameters* getComponentParameters(int componentId);
    
private:
    void setupLayout();
    void relayoutParams();

signals:
    void parameterEdited(int componentId, ParameterType p, ParameterValue value);

private slots:
    void onCloseButtonClicked();
};

#endif // GROUP_EDITOR_HPP_
