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
#include "types/CollectionType.hpp"
#include "util/util.hpp"
#include "widgets/NoteWidget.hpp"
#include "types/CollectionRequest.hpp"

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

void PianoRollWidget::removeNote(int idx){
    auto it = notes_.find(idx);
    if ( it == notes_.end()){
        qWarning() << "received request to delete note with index " << idx << ", but idx is not in map";
        return ;
    }

    delete it->second ;
    notes_.erase(it);
}

void PianoRollWidget::paintEvent(QPaintEvent*){
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);

    drawGrid(p);
    drawPianoKeys(p);
}

void PianoRollWidget::mousePressEvent(QMouseEvent* e){
    if ( e->button() == Qt::LeftButton ){
        qDebug() << "Press pos:" << e->pos() << "global:" << e->globalPosition() ;
        float beat = xToBeat(e->pos().x());
        uint8_t pitch = yToPitch(e->pos().y());
        qDebug() << "Calculated beat:" << beat << "pitch:" << pitch ;

        dragNote_ = new NoteWidget(pitch,100,beat,beat,this);
        qDebug() << "Created note at geometry:" << dragNote_->geometry();
        connect(dragNote_, &NoteWidget::noteClicked, this, &PianoRollWidget::onNoteClicked);
        isDragging_ = true ;
    }
}

void PianoRollWidget::mouseMoveEvent(QMouseEvent* e){
    if ( isDragging_ ){
        qDebug() << "Move pos:" << e->pos() << "global:" << e->globalPosition() ;
        QPointF pos = mapFromGlobal(e->globalPosition());
        float endBeat = xToBeat(pos.x());
        qDebug() << "End Beat:" << endBeat ;
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
        CollectionRequest req ;
        req.collectionType = CollectionType::SEQUENCER ;
        req.action = CollectionAction::ADD ;
        req.componentId = id_ ;
        req.value = dragNote_->getNote() ;
        
        QJsonObject obj = Util::nlohmannToQJsonObject(req);
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
    int width = Theme::PIANO_ROLL_KEY_WIDTH + static_cast<int>(totalBeats_ * Theme::PIANO_ROLL_PIXELS_PER_BEAT);
    int height = 128 * Theme::PIANO_ROLL_NOTE_HEIGHT ;
    setMinimumSize(width,height);
    setMaximumSize(width,height);
}

void PianoRollWidget::drawGrid(QPainter& p){
    // vertical beats
    p.setPen(QPen(Theme::PIANO_ROLL_BACKGROUND, Theme::PIANO_ROLL_GRID_PEN_WIDTH_PRIMARY));
    for ( int beat = 0; beat <= totalBeats_ ; ++beat ){
        int x = Theme::PIANO_ROLL_KEY_WIDTH + beat * Theme::PIANO_ROLL_PIXELS_PER_BEAT ;
        p.drawLine(x, 0, x, height());

    }

    // vertical beat subdivisions
    for ( float beat = 0 ; beat <= totalBeats_ ; beat += 0.25f ){
        if ( std::fmod(beat, 1.0) < .0001 ){
            p.setPen(QPen(Theme::PIANO_ROLL_GRID_PRIMARY, Theme::PIANO_ROLL_GRID_PEN_WIDTH_SECONDARY));
        } else {
            p.setPen(QPen(Theme::PIANO_ROLL_GRID_SECONDARY, Theme::PIANO_ROLL_GRID_PEN_WIDTH_SECONDARY));
        }
        int x = Theme::PIANO_ROLL_KEY_WIDTH + beat * Theme::PIANO_ROLL_PIXELS_PER_BEAT ;
        p.drawLine(x,0,x,height());
    }

    // horizontal notes
    for ( uint8_t note = 0 ; note < 128; ++note ){
        int y = note * Theme::PIANO_ROLL_NOTE_HEIGHT ;
        QColor keyColor ;
        if ( isWhiteNote(127 - note) ){
            keyColor = Theme::PIANO_ROLL_KEY_WHITE ;
        } else {
            keyColor = Theme::PIANO_ROLL_KEY_BLACK ;
        }
        p.setPen(QPen(keyColor, Theme::PIANO_ROLL_GRID_PEN_WIDTH_PRIMARY));
    }
}

void PianoRollWidget::drawPianoKeys(QPainter& p){
    for ( uint8_t note = 0; note < 128; ++note ){
        int y = (127 - note) * Theme::PIANO_ROLL_NOTE_HEIGHT ;
        QColor keyColor ;
        if ( isWhiteNote(note) ){
            keyColor = Theme::PIANO_ROLL_KEY_WHITE ;
        } else {
            keyColor = Theme::PIANO_ROLL_KEY_BLACK ;
        }
        p.fillRect(0,y,Theme::PIANO_ROLL_KEY_WIDTH, Theme::PIANO_ROLL_NOTE_HEIGHT, keyColor);
        p.setPen(Theme::PIANO_ROLL_KEY_BORDER);
        p.drawRect(0,y,Theme::PIANO_ROLL_KEY_WIDTH, Theme::PIANO_ROLL_NOTE_HEIGHT);

        // draw some note names
        if ( note % 12 == 0 ){
            p.setPen(Theme::PIANO_ROLL_KEY_LABEL);
            p.drawText(
                QRect(2,y, Theme::PIANO_ROLL_KEY_WIDTH - Theme::PIANO_ROLL_KEY_LABEL_X_PAD, Theme::PIANO_ROLL_NOTE_HEIGHT),
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

float PianoRollWidget::xToBeat(float x) const {
    return static_cast<float>(x - Theme::PIANO_ROLL_KEY_WIDTH) / Theme::PIANO_ROLL_PIXELS_PER_BEAT ;
}

int PianoRollWidget::yToPitch(float y) const {
    return 127 - static_cast<int>((y + Theme::PIANO_ROLL_NOTE_HEIGHT / 2.0) / Theme::PIANO_ROLL_NOTE_HEIGHT);
}

void PianoRollWidget::onApiDataReceived(const QJsonObject& json){
    QString action = json["action"].toString();

    if ( action == "add_collection_value" ){
        if ( json["status"] != "success" ){
            qDebug() << "sequence note was not successfully added." ;
            return ;
        }

        QJsonDocument doc = QJsonDocument(json["value"].toObject());
        SequenceNote note = nlohmann::json::parse(doc.toJson().toStdString());
        int index = json["index"].toInt();

        if ( dragNote_ && dragNote_->getNote() == note ){
            notes_[index] = dragNote_;
            dragNote_ = nullptr ;
        } else {
            notes_[index] = new NoteWidget(note, this);
            connect(notes_[index], &NoteWidget::noteClicked, this, &PianoRollWidget::onNoteClicked);
        }
        
        update();
        return ;
    }

    if ( action == "remove_collection_value" ){
        if ( json["status"] != "success" ){
            qDebug() << "sequence note was not successfully removed." ;
            return ;
        }

        QJsonDocument doc = QJsonDocument(json["value"].toObject());
        int index = json["index"].toInt();
        
        removeNote(index);
        update();
        return ;
    }
}

void PianoRollWidget::onNoteClicked(NoteWidget* note, bool multiSelect){
    if ( !multiSelect ){
        for ( int idx : selectedNotes_ ){
            auto it = notes_.find(idx);
            if ( it != notes_.end() && it->second ){
                it->second->setSelected(false);
            }
        }
        selectedNotes_.clear();
    }

    int idx = findNoteIndex(note);
    if ( !note || idx == -1 ){
        qDebug() << "clicked note does not have a valid index. Ignoring event.";
        return ;
    }

    if ( note->isSelected() ){    
        note->setSelected(false);
        selectedNotes_.erase(std::remove(selectedNotes_.begin(), selectedNotes_.end(), idx), selectedNotes_.end());
    } else {
        note->setSelected(true);
        selectedNotes_.push_back(idx);
    }
}

void PianoRollWidget::deleteSelectedNotes(){
    for ( int idx : selectedNotes_ ){
        CollectionRequest req ;
        req.collectionType = CollectionType::SEQUENCER ;
        req.action = CollectionAction::REMOVE ;
        req.index = idx ;
        req.componentId = id_ ;
        
        QJsonObject obj = Util::nlohmannToQJsonObject(req);
        ApiClient::instance()->sendMessage(obj);
    }
}

int PianoRollWidget::findNoteIndex(NoteWidget* note) const {
    for (const auto& [idx, widget] : notes_) {
        if (widget == note) {
            return idx;
        }
    }
    return -1;  // Not found
}