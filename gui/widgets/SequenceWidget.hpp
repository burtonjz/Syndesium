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

#ifndef SEQUENCE_WIDGET_HPP_
#define SEQUENCE_WIDGET_HPP_

#include "types/SequenceData.hpp"

#include <QWidget>
#include <QScrollArea>
#include <QPainter>
#include <QMouseEvent>
#include <qtmetamacros.h>
#include <vector>
#include <cstdint>

class SequenceWidget : public QWidget {
    Q_OBJECT

private:
    std::vector<SequenceNote> notes_ ;
    int id_ ;

    float totalBeats_ ;

    bool isDragging_ = false ;
    float dragStart_ = 0.0f ;
    uint8_t dragPitch_ = 0 ;

    static constexpr qreal SEQUENCE_NOTE_HEIGHT = 12 ;
    static constexpr qreal SEQUENCE_PIXELS_PER_BEAT = 48 ;
    static constexpr qreal SEQUENCE_KEY_WIDTH = 60 ;

    static constexpr qreal SEQUENCE_MAIN_GRID_PEN_WIDTH = 1 ;
    static constexpr qreal SEQUENCE_SUB_GRID_PEN_WIDTH = .75 ;

    static constexpr qreal SEQUENCE_KEY_LABEL_X_PAD = 4 ;

public:
    explicit SequenceWidget(int id, QWidget* parent = nullptr);

    void setTotalBeats(float beats);

    void setNotes(const std::vector<SequenceNote>& notes);
    const std::vector<SequenceNote>& getNotes() const ;

    void removeNote(SequenceNote note);

protected:
    void paintEvent(QPaintEvent*) override ;
    void mousePressEvent(QMouseEvent* e) override ;
    void mouseMoveEvent(QMouseEvent* e) override ;
    void mouseReleaseEvent(QMouseEvent* e) override ;

private:
    void updateSize();
    void drawGrid(QPainter& p);
    void drawPianoKeys(QPainter& p);
    void drawNotes(QPainter& p);

    bool isWhiteNote(uint8_t pitch) const ;
    float xToBeat(int x) const ;
    int yToPitch(int y) const ;

    QJsonObject nlohmannToQJsonObject(SequenceNote note) const ;

    // functions for api responses
    void onNoteAdded(SequenceNote note);
    void onNoteRemoved(SequenceNote note);
    
public slots:
    void onApiDataReceived(const QJsonObject &json);
};

#endif // SEQUENCE_WIDGET_HPP_