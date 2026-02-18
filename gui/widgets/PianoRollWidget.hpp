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

#include "widgets/CollectionWidget.hpp"
#include "models/ComponentModel.hpp"
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

class PianoRollWidget : public CollectionWidget {
    Q_OBJECT

private:
    std::map<int, NoteWidget*> notes_ ;
    std::vector<int> selectedNotes_ ;

    float totalBeats_ ;

    bool isDragging_ = false ;
    bool isResizing_ = false ;

    NoteWidget* dragNote_ ;
    float anchorBeat_ ;

public:
    explicit PianoRollWidget(ComponentModel* model, QWidget* parent = nullptr);

    void updateCollection(const CollectionRequest& req) override ;
    
    void setTotalBeats(float beats);
    // void removeNote(int idx);

protected:
    void paintEvent(QPaintEvent*) override ;
    void mousePressEvent(QMouseEvent* e) override ;
    void mouseMoveEvent(QMouseEvent* e) override ;
    void mouseReleaseEvent(QMouseEvent* e) override ;
    void keyPressEvent(QKeyEvent* e) override ;

private:
    // draw functions
    void updateSize();
    void drawGrid(QPainter& p);
    void drawPianoKeys(QPainter& p);

    bool isWhiteNote(uint8_t pitch) const ;
    float xToBeat(float x) const ;
    int yToPitch(float y) const ;

    // note handling
    int findNoteIndex(NoteWidget* note) const ;
    NoteWidget* findNoteAtPos(const QPointF& pos);
    
    // functions for Qt event handling
    void selectNote(NoteWidget* note, bool multiSelect);
    void deselectNotes();
    void handleNoteHover(const QPointF pos);

    // dragging new notes
    void startDrag(const QPointF pos);
    void updateDrag(const QPointF pos);
    void endDrag(const QPointF pos);

    // resize existing note
    bool startResize(NoteWidget* note, const QPointF pos);
    void updateResize(const QPointF pos);
    void endResize(const QPointF pos);

    void updateSelectedNotePitch(int p);
    void updateSelectedNoteStart(float t);
    void updateSelectedNoteDuration(float d);

    // functions for api responses
    void handleCollectionAdd(const CollectionRequest& req);
    void handleCollectionRemove(const CollectionRequest& req);
    
    void onNoteAdded(SequenceNote note);
    void onNoteRemoved(SequenceNote note);

    void requestRemoveNote(int idx);
    void requestRemoveSelectedNotes();
    
public slots:
    // respond to parameter changes from the component model
    void onParameterChanged(ParameterType p);
};

#endif // PIANOROLL_WIDGET_HPP_