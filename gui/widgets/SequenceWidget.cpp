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

#include "widgets/SequenceWidget.hpp"
#include "core/Theme.hpp"
#include "core/ApiClient.hpp"

#include <nlohmann/json.hpp>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <qjsondocument.h>

SequenceWidget::SequenceWidget(int id, QWidget* parent):
    QWidget(parent),
    id_(id),
    totalBeats_(16.0f)
{
    setMouseTracking(true);
    updateSize();

    connect(ApiClient::instance(), &ApiClient::dataReceived, this, &SequenceWidget::onApiDataReceived);
}

void SequenceWidget::setTotalBeats(float beats){
    totalBeats_ = beats ;
    updateSize();
    update();
}

void SequenceWidget::setNotes(const std::vector<SequenceNote>& notes){
    notes_ = notes ;
    update();
}

const std::vector<SequenceNote>& SequenceWidget::getNotes() const {
    return notes_ ;
}

void SequenceWidget::removeNote(SequenceNote note){
    auto it = std::find(notes_.begin(), notes_.end(), note) ;
    if ( it != notes_.end() ){
        notes_.erase(it);
    }
}

void SequenceWidget::paintEvent(QPaintEvent*){
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);

    drawGrid(p);
    drawPianoKeys(p);
    drawNotes(p);
}

void SequenceWidget::mousePressEvent(QMouseEvent* e){
    if ( e->button() == Qt::LeftButton ){
        float beat = xToBeat(e->pos().x());
        uint8_t pitch = yToPitch(e->pos().y());

        // if clicking on an existing note, it will delete it
        for ( const auto& n : notes_ ){
            if ( 
                n.pitch == pitch &&
                beat >= n.startBeat &&
                beat <= n.endBeat
            ){
                QJsonObject obj ;

                obj["action"] = "remove_sequence_note" ;
                obj["componentId"] = id_ ;
                obj["note"] = nlohmannToQJsonObject(n);
                ApiClient::instance()->sendMessage(obj); 
            }
        }

        // otherwise, start dragging new note
        dragStart_ = beat ;
        dragPitch_ = pitch ;
        isDragging_ = true ;
    }
}

void SequenceWidget::mouseMoveEvent(QMouseEvent* e){
    if ( isDragging_ ){
        update() ;
    }
}

void SequenceWidget::mouseReleaseEvent(QMouseEvent* e){
    if ( isDragging_ && e->button() == Qt::LeftButton ){
        float endBeat = xToBeat(e->pos().x());
        if ( endBeat < dragStart_ ) std::swap(dragStart_, endBeat);

        // 16th note snap
        dragStart_ = std::round(dragStart_ * 4) / 4.0f ;
        endBeat = std::round(endBeat * 4) / 4.0f ;

        if ( endBeat > dragStart_ ){
            SequenceNote note ;
            note.pitch = dragPitch_ ;
            note.startBeat = dragStart_ ;
            note.endBeat = endBeat ;
            
            QJsonObject obj ;

            // convert note to qObject
            obj["action"] = "add_sequence_note" ;
            obj["componentId"] = id_ ;
            obj["note"] = nlohmannToQJsonObject(note);
            ApiClient::instance()->sendMessage(obj); 
        }

        isDragging_ = false ;
    }
}

void SequenceWidget::updateSize(){
    int width = SEQUENCE_KEY_WIDTH + static_cast<int>(totalBeats_ * SEQUENCE_PIXELS_PER_BEAT);
    int height = 128 * SEQUENCE_NOTE_HEIGHT ;
    setMinimumSize(width,height);
    setMaximumSize(width,height);
}

void SequenceWidget::drawGrid(QPainter& p){
    // vertical beats
    p.setPen(QPen(Theme::BACKGROUND_LIGHT, SEQUENCE_MAIN_GRID_PEN_WIDTH));
    for ( int beat = 0; beat <= totalBeats_ ; ++beat ){
        int x = SEQUENCE_KEY_WIDTH + beat * SEQUENCE_PIXELS_PER_BEAT ;
        p.drawLine(x, 0, x, height());

    }

    // vertical beat subdivisions
    p.setPen(QPen(Theme::BACKGROUND_MEDIUM, SEQUENCE_SUB_GRID_PEN_WIDTH));
    for ( float beat = 0 ; beat <= totalBeats_ ; beat += 0.25f ){
        int x = SEQUENCE_KEY_WIDTH + beat * SEQUENCE_PIXELS_PER_BEAT ;
        p.drawLine(x,0,x,height());
    }

    // horizontal notes
    for ( uint8_t note = 0 ; note < 128; ++note ){
        int y = note * SEQUENCE_NOTE_HEIGHT ;
        QColor keyColor ;
        if ( isWhiteNote(127 - note) ){
            keyColor = Theme::BACKGROUND_MEDIUM ;
        } else {
            keyColor = Theme::BACKGROUND_DARK ;
        }
        p.setPen(QPen(keyColor, SEQUENCE_MAIN_GRID_PEN_WIDTH));
    }
}

void SequenceWidget::drawPianoKeys(QPainter& p){
    for ( uint8_t note = 0; note < 128; ++note ){
        int y = (127 - note) * SEQUENCE_NOTE_HEIGHT ;
        QColor keyColor ;
        if ( isWhiteNote(note) ){
            keyColor = Theme::PIANO_ROLL_KEY_WHITE ;
        } else {
            keyColor = Theme::PIANO_ROLL_KEY_BLACK ;
        }
        p.fillRect(0,y,SEQUENCE_KEY_WIDTH, SEQUENCE_NOTE_HEIGHT, keyColor);
        p.setPen(Theme::PIANO_ROLL_KEY_BORDER);
        p.drawRect(0,y,SEQUENCE_KEY_WIDTH, SEQUENCE_NOTE_HEIGHT);

        // draw some note names
        if ( note % 12 == 0 ){
            p.setPen(Theme::PIANO_ROLL_KEY_LABEL);
            p.drawText(
                QRect(2,y, SEQUENCE_KEY_WIDTH - SEQUENCE_KEY_LABEL_X_PAD, SEQUENCE_NOTE_HEIGHT),
                Qt::AlignCenter,
                QString("C%1").arg(note / 12 - 1)
            );
        }
    }
}   

void SequenceWidget::drawNotes(QPainter& p){
    // existing notes
    for ( const auto& note : notes_ ){
        int x = SEQUENCE_KEY_WIDTH + note.startBeat * SEQUENCE_PIXELS_PER_BEAT ;
        int y = (127 - note.pitch) * SEQUENCE_NOTE_HEIGHT ;
        int w = static_cast<int>(note.endBeat - note.startBeat) * SEQUENCE_PIXELS_PER_BEAT ;

        p.fillRect(x,y,w,SEQUENCE_NOTE_HEIGHT, Theme::PIANO_ROLL_NOTE_COLOR);
        p.setPen(Theme::PIANO_ROLL_NOTE_BORDER);
        p.drawRect(x, y, w, SEQUENCE_NOTE_HEIGHT);
    }

    // now the drag note
    if ( isDragging_ ){
        int x1 = SEQUENCE_KEY_WIDTH + dragStart_ * SEQUENCE_PIXELS_PER_BEAT ;
        int x2 = QCursor::pos().x() - mapToGlobal(QPoint(0,0)).x();
        int y = ( 127 - dragPitch_ ) * SEQUENCE_NOTE_HEIGHT ;
        int w = x2 - x1 ;

        p.fillRect(x1,y,w,SEQUENCE_NOTE_HEIGHT, Theme::PIANO_ROLL_NOTE_CREATION_COLOR);
        p.setPen(Theme::PIANO_ROLL_NOTE_BORDER);
        p.drawRect(x1, y, w, SEQUENCE_NOTE_HEIGHT);
    }
}

bool SequenceWidget::isWhiteNote(uint8_t pitch) const {
    uint8_t note = pitch % 12 ;
    return note == 0 || note == 2 || 
           note == 4 || note == 5 || 
           note == 7 || note == 9 || 
           note == 11 ;
}

float SequenceWidget::xToBeat(int x) const {
    return static_cast<float>(x - SEQUENCE_KEY_WIDTH) / SEQUENCE_PIXELS_PER_BEAT ;
}

int SequenceWidget::yToPitch(int y) const {
    return 127 - (y / SEQUENCE_NOTE_HEIGHT);
}

QJsonObject SequenceWidget::nlohmannToQJsonObject(SequenceNote note) const {
    nlohmann::json nj = note ;
    QString jsonStr = QString::fromStdString(nj.dump());
    QJsonDocument doc = QJsonDocument::fromJson(jsonStr.toUtf8());
    return doc.object() ;
}

void SequenceWidget::onApiDataReceived(const QJsonObject& json){
    QString action = json["action"].toString();

    if ( action == "add_sequence_note" ){
        if ( json["status"] != "success" ){
            qDebug() << "sequence note was not successfully added." ;
            return ;
        }

        QJsonDocument doc = QJsonDocument(json["note"].toObject());
        SequenceNote note = nlohmann::json::parse(doc.toJson().toStdString());
        notes_.push_back(note);
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