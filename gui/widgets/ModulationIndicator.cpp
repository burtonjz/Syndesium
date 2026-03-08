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

#include "widgets/ModulationIndicator.hpp"
#include "app/Theme.hpp"

#include <QPainter>
#include <QPen>

ModulationIndicator::ModulationIndicator(QWidget* parent): 
    QWidget(parent), active_(false)
{
    setFixedSize(Theme::MODULATION_INDICATOR_SIZE, Theme::MODULATION_INDICATOR_SIZE);
    updateStyle();
}

void ModulationIndicator::setActive(bool active){
    if ( active_ == active ) return;
    active_ = active ;
    updateStyle();
}

bool ModulationIndicator::isActive() const { 
    return active_ ; 
}

void ModulationIndicator::updateStyle(){
    if ( active_ ){
        setStyleSheet(QString(
            "background: %1;"
            "border-radius: %2px;"
            "box-shadow: 0 0 6px 3px %1;"
        ).arg(Theme::MODULATION_ACTIVE.name())
         .arg(Theme::MODULATION_INDICATOR_SIZE / 2));
    } else {
        setStyleSheet(QString(
            "background: %1;"
            "border-radius: %2px;"
        ).arg(Theme::MODULATION_INACTIVE.name())
         .arg(Theme::MODULATION_INDICATOR_SIZE / 2));
    }
}