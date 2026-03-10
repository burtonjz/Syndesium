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

#ifndef MODULATION_INDICATOR_HPP_
#define MODULATION_INDICATOR_HPP_

#include <QWidget>
class ModulationIndicator : public QWidget {
    Q_OBJECT

private:
    bool active_;

public:
    explicit ModulationIndicator(QWidget* parent = nullptr);
    bool isActive() const ;

protected:
    void paintEvent(QPaintEvent* e) override ;
    QSize sizeHint() const override ;

public slots:
    void setActive(bool active);

};


#endif // MODULATION_INDICATOR_HPP_