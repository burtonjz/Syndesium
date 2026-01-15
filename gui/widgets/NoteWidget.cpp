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

#include "widgets/NoteWidget.hpp"
#include "widgets/PianoRollWidget.hpp"
#include "core/Theme.hpp"
#include <QEvent>
#include <QWidget>

NoteWidget::NoteWidget(uint8_t midiNote, uint8_t velocity, float start, float end, PianoRollWidget* parent):
    QWidget(parent),
    midiNote_(midiNote),
    velocity_(velocity),
    startBeat_(start),
    endBeat_(end),
    selected_(false),
    noteName_(midi2str(midiNote))
{ 
    setToolTip(noteName_);
    setMouseTracking(true);
    updateSize();
}

NoteWidget::NoteWidget(SequenceNote note, PianoRollWidget* parent):
    NoteWidget::NoteWidget(note.pitch, note.velocity, note.startBeat, note.getEndBeat(), parent)
{}

void NoteWidget::paintEvent(QPaintEvent*){
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    QColor fillColor = selected_ ? Theme::PIANO_ROLL_NOTE_SELECTED_COLOR :
        Theme::PIANO_ROLL_NOTE_COLOR ;

    painter.fillRect(0,0,w_, Theme::PIANO_ROLL_NOTE_HEIGHT, fillColor);
    painter.setPen(Theme::PIANO_ROLL_NOTE_BORDER);
    painter.drawRect(0,0,w_, Theme::PIANO_ROLL_NOTE_HEIGHT);
}

void NoteWidget::mousePressEvent(QMouseEvent* e){
    if ( e->button() == Qt::LeftButton ){
        bool multiSelect = e->modifiers() & Qt::ControlModifier ;
        emit noteClicked(this, multiSelect);
        e->accept();
    }
}

uint8_t NoteWidget::getMidiNote() const {
    return midiNote_ ;
}

void NoteWidget::setMidiNote(uint8_t midiNote){
    midiNote_ = std::min(std::max(midiNote, (uint8_t) 0), (uint8_t) 127 );
    updateSize();
}

uint8_t NoteWidget::getVelocity() const {
    return velocity_ ;
}

void NoteWidget::setVelocity(uint8_t velocity){
    velocity_ = std::min(std::max(velocity, (uint8_t) 0), (uint8_t) 127 );
    updateSize();
}

float NoteWidget::getStartBeat() const {
    return startBeat_ ;
}

void NoteWidget::setStartBeat(float startBeat, bool round){
    if ( round ){
        startBeat_ = std::round(startBeat * 4) / 4.0f ;
    } else {
        startBeat_ = startBeat ;
    }
    updateSize();
}

float NoteWidget::getEndBeat() const {
    return endBeat_ ;
}

void NoteWidget::setEndBeat(float endBeat, bool round){
    if ( round ){
        endBeat_ = std::round(endBeat * 4) / 4.0f ;
    } else {
        endBeat_ = endBeat ;
    }
    updateSize();
}

SequenceNote NoteWidget::getNote() const {
    return {midiNote_,velocity_,startBeat_,endBeat_ - startBeat_};
}

void NoteWidget::setSelected(bool selected){
    selected_ = selected ;
    update();
}

bool NoteWidget::isSelected() const {
    return selected_ ;
}

QString NoteWidget::midi2str(uint8_t midiNote){
    QString note ;
    switch( midiNote % 12 ){
        case 0 : note = "C"  ; break ;
        case 1 : note = "C#" ; break ;
        case 2 : note = "D"  ; break ;
        case 3 : note = "D#" ; break ;
        case 4 : note = "E"  ; break ;
        case 5 : note = "F"  ; break ;
        case 6 : note = "F#" ; break ;
        case 7 : note = "G"  ; break ;
        case 8 : note = "G#" ; break ;
        case 9 : note = "A"  ; break ;
        case 10: note = "A#" ; break ;
        case 11: note = "B"  ; break ;
    }

    return note + QString("%1").arg(midiNote / 12 - 1) ;
}

void NoteWidget::updateSize(){
    x_ = Theme::PIANO_ROLL_KEY_WIDTH + startBeat_ * 
        Theme::PIANO_ROLL_PIXELS_PER_BEAT ;
    y_ = (127 - midiNote_) * Theme::PIANO_ROLL_NOTE_HEIGHT ;
    w_ = static_cast<int>(endBeat_ - startBeat_) * Theme::PIANO_ROLL_PIXELS_PER_BEAT ;
    setGeometry(x_,y_,w_, Theme::PIANO_ROLL_NOTE_HEIGHT);
    show();
    update();
    qDebug() << "updating note geometry to " << geometry() ;
}