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

#ifndef PIANOROLL_WIDGET_HPP_
#define PIANOROLL_WIDGET_HPP_

#include "types/SequenceData.hpp"
#include "widgets/NoteWidget.hpp"

#include <QWidget>
#include <QScrollArea>
#include <QPainter>
#include <QMouseEvent>
#include <QKeyEvent>
#include <vector>
#include <map>
#include <cstdint>

class PianoRollWidget : public QWidget {
    Q_OBJECT

private:
    std::map<int, NoteWidget*> notes_ ;
    std::vector<int> selectedNotes_ ;
    int id_ ;

    float totalBeats_ ;

    bool isDragging_ = false ;
    NoteWidget* dragNote_ ;

    static constexpr qreal NOTE_HEIGHT = 30 ;
    static constexpr qreal PIXELS_PER_BEAT = 48 ;
    static constexpr qreal KEY_WIDTH = 60 ;
    static constexpr qreal MAIN_GRID_PEN_WIDTH = 1 ;
    static constexpr qreal SUB_GRID_PEN_WIDTH = .75 ;
    static constexpr qreal KEY_LABEL_X_PAD = 4 ;

public:
    explicit PianoRollWidget(int id, QWidget* parent = nullptr);

    void setTotalBeats(float beats);
    void removeNote(int idx);

protected:
    void paintEvent(QPaintEvent*) override ;
    void mousePressEvent(QMouseEvent* e) override ;
    void mouseMoveEvent(QMouseEvent* e) override ;
    void mouseReleaseEvent(QMouseEvent* e) override ;
    void keyPressEvent(QKeyEvent* e) override ;

private:
    int findNoteIndex(NoteWidget* note) const ;
    
    void updateSize();
    void drawGrid(QPainter& p);
    void drawPianoKeys(QPainter& p);

    bool isWhiteNote(uint8_t pitch) const ;
    float xToBeat(int x) const ;
    int yToPitch(int y) const ;

    // functions for api responses
    void onNoteAdded(SequenceNote note);
    void onNoteRemoved(SequenceNote note);
    void deleteSelectedNotes();
    
public slots:
    void onApiDataReceived(const QJsonObject &json);
    void onNoteClicked(NoteWidget* note, bool multiSelect);
};

#endif // PIANOROLL_WIDGET_HPP_