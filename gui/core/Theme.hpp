/*
 * Copyright (C) 2025 Jared Burton
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY ; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program. If not, see <https://www.gnu.org/licenses/>.
 */

 #ifndef __UI_THEME_HPP_
 #define __UI_THEME_HPP_

#include <QColor>
#include <QString>
#include <QApplication>
#include <QPalette>

class Theme {
public:
    static const QColor GRAPH_GRID_COLOR ;

    // Cable/Connection Colors
    static const QColor CABLE_SHADOW ;
    static const QColor CABLE_AUDIO ;
    static const QColor CABLE_MODULATION ;
    static const QColor CABLE_MIDI ;
    
    // Socket Colors
    static const QColor SOCKET_AUDIO ;
    static const QColor SOCKET_AUDIO_LIGHT ;
    static const QColor SOCKET_MIDI ;
    static const QColor SOCKET_MIDI_LIGHT ;
    static const QColor SOCKET_MODULATION ;
    static const QColor SOCKET_MODULATION_LIGHT ;

    // Component Colors
    static const QColor COMPONENT_BORDER ;
    static const QColor COMPONENT_BORDER_SELECTED ;
    static const QColor COMPONENT_BACKGROUND ;
    static const QColor COMPONENT_BACKGROUND_HOVER ;
    static const QColor COMPONENT_TEXT ;
    
    // UI Colors
    static const QColor BACKGROUND_DARK ;
    static const QColor BACKGROUND_MEDIUM ;
    static const QColor BACKGROUND_LIGHT ;
    static const QColor TEXT_PRIMARY ;
    static const QColor TEXT_SECONDARY ;
    static const QColor ACCENT_COLOR ;
    
    // Piano Roll
    static const QColor PIANO_ROLL_KEY_WHITE ;
    static const QColor PIANO_ROLL_KEY_BLACK ;
    static const QColor PIANO_ROLL_KEY_BORDER ;
    static const QColor PIANO_ROLL_KEY_LABEL ;
    static const QColor PIANO_ROLL_NOTE_COLOR ;
    static const QColor PIANO_ROLL_NOTE_SELECTED_COLOR ;
    static const QColor PIANO_ROLL_NOTE_BORDER ;

    // Modulation indicator
    static const QColor MODULATION_ACTIVE ;
    
    // Apply theme to application
    static void applyDarkTheme();
    
    // Get stylesheet snippets
    static QString getComponentStyle(bool selected = false, bool hovered = false);
    static QString getModulationStyle();
};

 #endif // __UI_THEME_HPP_

