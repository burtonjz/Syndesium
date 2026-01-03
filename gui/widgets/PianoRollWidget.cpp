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

#include "widgets/PianoRollWidget.hpp"
#include "core/Theme.hpp"
#include "core/ApiClient.hpp"
#include "util/util.hpp"
#include "widgets/NoteWidget.hpp"

#include <nlohmann/json.hpp>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <qjsondocument.h>
#include <qpainter.h>

PianoRollWidget::PianoRollWidget(int id, QWidget* parent):
    QWidget(parent),
    id_(id),
    totalBeats_(16.0f),
    notes_(),
    selectedNotes_()
{
    setMouseTracking(true);
    setFocusPolicy(Qt::StrongFocus);

    updateSize();
    
    connect(ApiClient::instance(), &ApiClient::dataReceived, this, &PianoRollWidget::onApiDataReceived);
}

void PianoRollWidget::setTotalBeats(float beats){
    totalBeats_ = beats ;
    updateSize();
    update();
}

void PianoRollWidget::setNotes(const std::vector<SequenceNote>& notes){
    if ( !notes_.empty() ){
        for ( auto n : notes_ ){
            delete n ;
        }
        notes_.clear();
    }

    for ( auto& note : notes ){
        notes_.push_back(new NoteWidget(note, this));
        connect(notes_.back(), &NoteWidget::noteClicked, this, &PianoRollWidget::onNoteClicked);
    }

    update();
}

const std::vector<SequenceNote> PianoRollWidget::getNotes() const {
    std::vector<SequenceNote> v ;
    for ( auto& n : notes_ ){
        v.push_back(n->getNote());
    }
    return v ;
}

void PianoRollWidget::removeNote(SequenceNote note){
    for ( auto it = notes_.begin(); it != notes_.end() ; ++it ){
        if ( *it && (*it)->getNote() == note ){
            delete *it ;
            notes_.erase(it);
            break ;
        }
    }
}

void PianoRollWidget::paintEvent(QPaintEvent*){
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);

    drawGrid(p);
    drawPianoKeys(p);
}

void PianoRollWidget::mousePressEvent(QMouseEvent* e){
    if ( e->button() == Qt::LeftButton ){
        float beat = xToBeat(e->pos().x());
        uint8_t pitch = yToPitch(e->pos().y());

        dragNote_ = new NoteWidget(pitch,100,beat,beat,this);
        connect(dragNote_, &NoteWidget::noteClicked, this, &PianoRollWidget::onNoteClicked);
        isDragging_ = true ;
    }
}

void PianoRollWidget::mouseMoveEvent(QMouseEvent* e){
    if ( isDragging_ ){
        float endBeat = xToBeat(e->pos().x());
        dragNote_->setEndBeat(endBeat);
    }
}

void PianoRollWidget::mouseReleaseEvent(QMouseEvent* e){
    if ( isDragging_ && e->button() == Qt::LeftButton ){
        float endBeat = xToBeat(e->pos().x());
        if ( endBeat < dragNote_->getStartBeat() ){
            dragNote_->setEndBeat(dragNote_->getStartBeat(), true);
            dragNote_->setStartBeat(endBeat, true);
        }  else {
            dragNote_->setStartBeat(dragNote_->getStartBeat(), true);
            dragNote_->setEndBeat(endBeat, true);
        }

        if ( dragNote_->getEndBeat() == dragNote_->getStartBeat() ){
            delete dragNote_ ;
            dragNote_ = nullptr ;
            isDragging_ = false ;
            return ;
        } 

        // convert note to qObject
        QJsonObject obj ;
        obj["action"] = "add_sequence_note" ;
        obj["componentId"] = id_ ;
        obj["note"] = Util::nlohmannToQJsonObject(dragNote_->getNote());
        ApiClient::instance()->sendMessage(obj); 

        isDragging_ = false ;
    }
}

void PianoRollWidget::keyPressEvent(QKeyEvent* e){
    qDebug() << "Key pressed:" << e->key() << "Text:" << e->text();
    if ( e->key() == Qt::Key_Delete ){
        deleteSelectedNotes();
    }
}

void PianoRollWidget::updateSize(){
    int width = KEY_WIDTH + static_cast<int>(totalBeats_ * PIXELS_PER_BEAT);
    int height = 128 * NOTE_HEIGHT ;
    setMinimumSize(width,height);
    setMaximumSize(width,height);
}

void PianoRollWidget::drawGrid(QPainter& p){
    // vertical beats
    p.setPen(QPen(Theme::BACKGROUND_LIGHT, MAIN_GRID_PEN_WIDTH));
    for ( int beat = 0; beat <= totalBeats_ ; ++beat ){
        int x = KEY_WIDTH + beat * PIXELS_PER_BEAT ;
        p.drawLine(x, 0, x, height());

    }

    // vertical beat subdivisions
    p.setPen(QPen(Theme::BACKGROUND_MEDIUM, SUB_GRID_PEN_WIDTH));
    for ( float beat = 0 ; beat <= totalBeats_ ; beat += 0.25f ){
        int x = KEY_WIDTH + beat * PIXELS_PER_BEAT ;
        p.drawLine(x,0,x,height());
    }

    // horizontal notes
    for ( uint8_t note = 0 ; note < 128; ++note ){
        int y = note * NOTE_HEIGHT ;
        QColor keyColor ;
        if ( isWhiteNote(127 - note) ){
            keyColor = Theme::BACKGROUND_MEDIUM ;
        } else {
            keyColor = Theme::BACKGROUND_DARK ;
        }
        p.setPen(QPen(keyColor, MAIN_GRID_PEN_WIDTH));
    }
}

void PianoRollWidget::drawPianoKeys(QPainter& p){
    for ( uint8_t note = 0; note < 128; ++note ){
        int y = (127 - note) * NOTE_HEIGHT ;
        QColor keyColor ;
        if ( isWhiteNote(note) ){
            keyColor = Theme::PIANO_ROLL_KEY_WHITE ;
        } else {
            keyColor = Theme::PIANO_ROLL_KEY_BLACK ;
        }
        p.fillRect(0,y,KEY_WIDTH, NOTE_HEIGHT, keyColor);
        p.setPen(Theme::PIANO_ROLL_KEY_BORDER);
        p.drawRect(0,y,KEY_WIDTH, NOTE_HEIGHT);

        // draw some note names
        if ( note % 12 == 0 ){
            p.setPen(Theme::PIANO_ROLL_KEY_LABEL);
            p.drawText(
                QRect(2,y, KEY_WIDTH - KEY_LABEL_X_PAD, NOTE_HEIGHT),
                Qt::AlignCenter,
                QString("C%1").arg(note / 12 - 1)
            );
        }
    }
}   

bool PianoRollWidget::isWhiteNote(uint8_t pitch) const {
    uint8_t note = pitch % 12 ;
    return note == 0 || note == 2 || 
           note == 4 || note == 5 || 
           note == 7 || note == 9 || 
           note == 11 ;
}

float PianoRollWidget::xToBeat(int x) const {
    return static_cast<float>(x - KEY_WIDTH) / PIXELS_PER_BEAT ;
}

int PianoRollWidget::yToPitch(int y) const {
    return 127 - (y / NOTE_HEIGHT);
}

void PianoRollWidget::onApiDataReceived(const QJsonObject& json){
    QString action = json["action"].toString();

    if ( action == "add_sequence_note" ){
        if ( json["status"] != "success" ){
            qDebug() << "sequence note was not successfully added." ;
            return ;
        }

        QJsonDocument doc = QJsonDocument(json["note"].toObject());
        SequenceNote note = nlohmann::json::parse(doc.toJson().toStdString());
        if ( dragNote_ && dragNote_->getNote() == note ){
            notes_.push_back(dragNote_);
            dragNote_ = nullptr ;
        } else {
            notes_.push_back(new NoteWidget(note, this));
            connect(notes_.back(), &NoteWidget::noteClicked, this, &PianoRollWidget::onNoteClicked);
        }
        
        update();
        return ;
    }

    if ( action == "remove_sequence_note" ){
        if ( json["status"] != "success" ){
            qDebug() << "sequence note was not successfully removed." ;
            return ;
        }

        QJsonDocument doc = QJsonDocument(json["note"].toObject());
        SequenceNote note = nlohmann::json::parse(doc.toJson().toStdString());
        removeNote(note);
        update();
        return ;
    }
}

void PianoRollWidget::onNoteClicked(NoteWidget* note, bool multiSelect){
    if ( !multiSelect ){
        for (auto* n : selectedNotes_ ){
            if (n) n->setSelected(false);
        }
        selectedNotes_.clear();
    }

    if ( note && note->isSelected() ){
        note->setSelected(false);
        selectedNotes_.erase(std::remove(selectedNotes_.begin(), selectedNotes_.end(), note), selectedNotes_.end());
    } else {
        note->setSelected(true);
        selectedNotes_.push_back(note);
    }
}

void PianoRollWidget::deleteSelectedNotes(){
    for ( auto* note : selectedNotes_ ){
        QJsonObject obj ;
        obj["action"] = "remove_sequence_note" ;
        obj["componentId"] = id_ ;
        obj["note"] = Util::nlohmannToQJsonObject(note->getNote());
        ApiClient::instance()->sendMessage(obj);
    }
}