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

#include "core/Theme.hpp"
#include <qcolor.h>
#include <qpalette.h>

const QColor Theme::GRAPH_GRID_COLOR = QColor(50,50,50);

// Cable colors
const QColor Theme::CABLE_SHADOW = QColor(0, 0, 0, 50);
const QColor Theme::CABLE_AUDIO = QColor(100, 255, 150); 
const QColor Theme::CABLE_MODULATION = QColor(255, 107, 107);
const QColor Theme::CABLE_MIDI = QColor(100, 200, 255);

// Socket colors
const QColor Theme::SOCKET_AUDIO = QColor(100, 255, 150); 
const QColor Theme::SOCKET_MODULATION = QColor(255, 107, 107);
const QColor Theme::SOCKET_MIDI = QColor(100, 200, 255);
const QColor Theme::SOCKET_AUDIO_LIGHT = Theme::SOCKET_AUDIO.lighter(150);
const QColor Theme::SOCKET_MODULATION_LIGHT = Theme::SOCKET_MODULATION.lighter(150);
const QColor Theme::SOCKET_MIDI_LIGHT = Theme::SOCKET_MIDI.lighter(150);

// Component colors
const QColor Theme::COMPONENT_BORDER = QColor(100, 100, 100);
const QColor Theme::COMPONENT_BORDER_SELECTED = QColor(220, 220, 220);
const QColor Theme::COMPONENT_BACKGROUND = QColor(45, 45, 45);
const QColor Theme::COMPONENT_BACKGROUND_HOVER = QColor(55, 55, 55);
const QColor Theme::COMPONENT_TEXT = QColor(250,250,250);

// UI colors
const QColor Theme::BACKGROUND_DARK = QColor(25, 25, 25);
const QColor Theme::BACKGROUND_MEDIUM = QColor(53, 53, 53);
const QColor Theme::BACKGROUND_LIGHT = QColor(70, 70, 70);
const QColor Theme::TEXT_PRIMARY = QColor(255, 255, 255);
const QColor Theme::TEXT_SECONDARY = QColor(180, 180, 180);
const QColor Theme::ACCENT_COLOR = QColor(100, 200, 255);
const QColor Theme::MODULATION_ACTIVE = QColor(255, 107, 107);

void Theme::applyDarkTheme() {
    QPalette darkPalette;
    darkPalette.setColor(QPalette::Window, BACKGROUND_MEDIUM);
    darkPalette.setColor(QPalette::WindowText, TEXT_PRIMARY);
    darkPalette.setColor(QPalette::Base, BACKGROUND_DARK);
    darkPalette.setColor(QPalette::AlternateBase, BACKGROUND_MEDIUM);
    darkPalette.setColor(QPalette::Text, TEXT_PRIMARY);
    darkPalette.setColor(QPalette::Button, BACKGROUND_MEDIUM);
    darkPalette.setColor(QPalette::ButtonText, TEXT_PRIMARY);
    darkPalette.setColor(QPalette::BrightText, Qt::red);
    darkPalette.setColor(QPalette::Link, ACCENT_COLOR);
    darkPalette.setColor(QPalette::Highlight, ACCENT_COLOR);
    darkPalette.setColor(QPalette::HighlightedText, Qt::black);
    
    qApp->setPalette(darkPalette);
    qApp->setStyle("Fusion");
    
    // Global stylesheet
    QString styleSheet = QString(R"(
    QWidget {
        font-size: 11pt;
    }
    
    QToolTip {
        background-color: %2;
        color: %5;
        border: 1px solid %1;
        padding: 5px;
        border-radius: 3px;
    }

    QMenu {
        background-color: %2;
        color: %5;
        border: 1px solid %1;
        padding: 5px;
        border-radius: 3px;
    }
    
    QGroupBox {
        font-weight: bold;
        border: 1px solid %1;
        border-radius: 5px;
        margin-top: 10px;
        padding-top: 10px;
        background-color: %2;
    }
    
    QGroupBox::title {
        subcontrol-origin: margin;
        left: 10px;
        padding: 0 5px;
    }
    
    QPushButton {
        padding: 5px 15px;
        border-radius: 3px;
        background-color: %2;
        border: 1px solid %1;
        color: %5;
    }
    
    QPushButton:hover {
        background-color: %3;
    }
    
    QDoubleSpinBox, QSpinBox, QComboBox {
        padding: 3px;
        background-color: %4;
        border: 1px solid %1;
        border-radius: 3px;
        color: %5;
    }
    
    QComboBox QAbstractItemView {
        background-color: %4;
        selection-background-color: %6;
        selection-color: %5;
        border: 2px solid %7;
        outline: none;
    }
    
    QComboBox QAbstractItemView::item {
        border-bottom: 1px solid %7;
        padding: 4px;
    }
    
    QLabel {
        color: %5;
    }
)").arg(COMPONENT_BORDER.name())              // %1
   .arg(BACKGROUND_MEDIUM.name())              // %2
   .arg(COMPONENT_BACKGROUND_HOVER.name())     // %3
   .arg(BACKGROUND_DARK.name())                // %4
   .arg(TEXT_PRIMARY.name())                   // %5
   .arg(ACCENT_COLOR.name())                   // %6
   .arg(BACKGROUND_LIGHT.name());              // %7 - brighter separator
    
    qApp->setStyleSheet(styleSheet);
}

QString Theme::getComponentStyle(bool selected, bool hovered) {
    QColor borderColor = selected ? COMPONENT_BORDER_SELECTED : COMPONENT_BORDER;
    QColor bgColor = hovered ? COMPONENT_BACKGROUND_HOVER : COMPONENT_BACKGROUND;
    
    return QString("background-color: %1; border: 2px solid %2;")
        .arg(bgColor.name())
        .arg(borderColor.name());
}

QString Theme::getModulationStyle() {
    return QString(
        "QDoubleSpinBox { "
        "  border: 2px solid %1; "
        "  background-color: rgba(%2, %3, %4, 30);"
        "}"
    ).arg(MODULATION_ACTIVE.name())
     .arg(MODULATION_ACTIVE.red())
     .arg(MODULATION_ACTIVE.green())
     .arg(MODULATION_ACTIVE.blue());
}
