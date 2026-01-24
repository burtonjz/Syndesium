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
    // we need to remove notes that aren't within the new bounds

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
    QPointF pos = mapFromGlobal(e->globalPosition());
    
    if ( e->button() == Qt::LeftButton ){
        NoteWidget* clickedNote = findNoteAtPos(pos);
        if ( clickedNote ){
            if ( onResizeStart(clickedNote, pos) ){
                e->accept();
                return ;
            } 

            bool multiSelect = e->modifiers() & Qt::ControlModifier ;
            onNoteSelected(clickedNote, multiSelect);
            e->accept();
            return ;
        }

        deselectNotes();
        onDragStart(pos);
        return ;
    }
}

void PianoRollWidget::mouseMoveEvent(QMouseEvent* e){
    QPointF pos = mapFromGlobal(e->globalPosition());

    if ( isDragging_ ){    
        onDragMove(pos);
        return ;
    }

    if ( isResizing_ ){
        onResizeMove(pos);
        return ;
    }

    onNoteHover(pos);
    
}

void PianoRollWidget::mouseReleaseEvent(QMouseEvent* e){
    QPointF pos = mapFromGlobal(e->globalPosition());

    if ( e->button() == Qt::LeftButton ){
         if ( isDragging_ ){
            onDragEnd(pos);
            e->accept();
            return ;
         }

         if ( isResizing_ ){
            onResizeEnd(pos);
            e->accept();
            return ;
         }
    }
}

void PianoRollWidget::keyPressEvent(QKeyEvent* e){
    if ( e->key() == Qt::Key_Delete ){
        deleteSelectedNotes();
        e->accept();
        return ;
    }

    if ( e->key() == Qt::Key_Up ){
        updateSelectedNotePitch(1);
        e->accept();
        return ;
    }
        
    if ( e->key() == Qt::Key_Down ){
        updateSelectedNotePitch(-1);
        e->accept();
        return ;
    }

    if ( e->key() == Qt::Key_Right ){
        if ( e->modifiers() & Qt::ControlModifier ){
            updateSelectedNoteDuration(0.125);
        } else {
            updateSelectedNoteStart(0.125);
        }        
        e->accept();
        return ;
    }

    if ( e->key() == Qt::Key_Left ){
        if ( e->modifiers() & Qt::ControlModifier ){
            updateSelectedNoteDuration(-0.125);
        } else {
            updateSelectedNoteStart(-0.125);
        }
        e->accept();
        return ;
    }
}

void PianoRollWidget::updateSize(){
    float width = Theme::PIANO_ROLL_KEY_WIDTH + static_cast<float>(totalBeats_ * Theme::PIANO_ROLL_PIXELS_PER_BEAT);
    float height = 128 * Theme::PIANO_ROLL_NOTE_HEIGHT ;
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
    float row = ( y - Theme::PIANO_ROLL_NOTE_HEIGHT / 2.0 ) / Theme::PIANO_ROLL_NOTE_HEIGHT ;
    return 127 - static_cast<int>(std::round(row));
}

void PianoRollWidget::onNoteSelected(NoteWidget* note, bool multiSelect){
    int idx = findNoteIndex(note);

    if ( !note || idx == -1 ){
        deselectNotes();
        return ;
    }

    if ( !multiSelect ){
        deselectNotes();
    }

    if ( note->isSelected() ){    
        note->setSelected(false);
        selectedNotes_.erase(std::remove(selectedNotes_.begin(), selectedNotes_.end(), idx), selectedNotes_.end());
    } else {
        note->setSelected(true);
        selectedNotes_.push_back(idx);
    }
}

void PianoRollWidget::deselectNotes(){
    for ( int idx : selectedNotes_ ){
        auto it = notes_.find(idx);
        if ( it != notes_.end() && it->second ){
            it->second->setSelected(false);
        }
    }
    selectedNotes_.clear();
}

void PianoRollWidget::onNoteHover(const QPointF pos){
    NoteWidget* n = findNoteAtPos(pos);
    if ( n ){
        QPointF notePos = n->mapFromParent(pos); // note local position
        const qreal threshold = Theme::PIANO_ROLL_NOTE_EDGE_THRESHOLD ;

        if ( notePos.x() <= threshold || notePos.x() >= n->width() - threshold ){
            setCursor(Qt::SizeHorCursor);
        } else {
            setCursor(Qt::ArrowCursor);
        }
    } else {
        setCursor(Qt::ArrowCursor);
    }
}

void PianoRollWidget::deleteNote(int idx){
    CollectionRequest req ;
    req.collectionType = CollectionType::SEQUENCER ;
    req.action = CollectionAction::REMOVE ;
    req.index = idx ;
    req.componentId = id_ ;
    
    QJsonObject obj = Util::nlohmannToQJsonObject(req);
    ApiClient::instance()->sendMessage(obj);
};

void PianoRollWidget::deleteSelectedNotes(){
    for ( int idx : selectedNotes_ ){
        deleteNote(idx);
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

NoteWidget* PianoRollWidget::findNoteAtPos(const QPointF& pos) {
    for ( auto& [idx, note] : notes_ ) {
        if (note && note->geometry().contains(pos.toPoint())) {
            return note;
        }
    }
    return nullptr;
}

void PianoRollWidget::onDragStart(const QPointF pos){
    anchorBeat_ = xToBeat(pos.x());
    uint8_t pitch = yToPitch(pos.y());
    dragNote_ = new NoteWidget(pitch,100,anchorBeat_ + 0.25,anchorBeat_,this);
    isDragging_ = true ;
}

void PianoRollWidget::onDragMove(const QPointF pos){
    float dragBeat = xToBeat(pos.x());
    dragNote_->setBeatRange(anchorBeat_, dragBeat);
}

void PianoRollWidget::onDragEnd(const QPointF pos){
    float dragBeat = xToBeat(pos.x());
    dragNote_->setBeatRange(anchorBeat_, dragBeat, true);

    if ( dragNote_->getEndBeat() == dragNote_->getStartBeat() ){
        delete dragNote_ ;
        dragNote_ = nullptr ;
        isDragging_ = false ;
        return ;
    } 

    // send note to api client
    CollectionRequest req ;
    req.collectionType = CollectionType::SEQUENCER ;
    req.action = CollectionAction::ADD ;
    req.componentId = id_ ;
    req.value = dragNote_->getNote() ;
    
    QJsonObject obj = Util::nlohmannToQJsonObject(req);
    ApiClient::instance()->sendMessage(obj); 

    isDragging_ = false ;
    return ;
}

bool PianoRollWidget::onResizeStart(NoteWidget* note, const QPointF pos){
    // if it's within edge threshold, start a resize
    QPointF notePos = note->mapFromParent(pos); // this is a local position
    const qreal threshold = Theme::PIANO_ROLL_NOTE_EDGE_THRESHOLD ;
    if ( notePos.x() <= threshold ){ // left side click
        isResizing_ = true ;
        anchorBeat_ = note->getEndBeat() ;
        dragNote_ = note ;
        return true ;
    } else if ( notePos.x() >= note->width() - threshold ){ // right side click
        isResizing_ = true ;
        anchorBeat_ = note->getStartBeat() ;
        dragNote_ = note ;
        return true ;
    }

    return false ;
}

void PianoRollWidget::onResizeMove(const QPointF pos){
    float dragBeat = xToBeat(pos.x());
    dragNote_->setBeatRange(anchorBeat_, dragBeat);
}

void PianoRollWidget::onResizeEnd(const QPointF pos){
    float dragBeat = xToBeat(pos.x());
    dragNote_->setBeatRange(anchorBeat_, dragBeat, true);


    int idx = findNoteIndex(dragNote_);
    
    // validate index
    if ( idx == -1 ){
        qWarning() << "attempted to delete a note that has no index. Please investigate" ;
        return ;
    }

    // delete note if no length
    if ( dragNote_->getEndBeat() == dragNote_->getStartBeat() ){
        deleteNote(idx);
        dragNote_ = nullptr ;
        isResizing_ = false ;
        return ;
    } 

    // otherwise, update note
    CollectionRequest req ;
    req.collectionType = CollectionType::SEQUENCER ;
    req.action = CollectionAction::SET ;
    req.componentId = id_ ;
    req.index =  idx ;
    req.value = dragNote_->getNote() ;
    
    QJsonObject obj = Util::nlohmannToQJsonObject(req);
    ApiClient::instance()->sendMessage(obj); 

    isResizing_ = false ;
    return ;
}

void PianoRollWidget::updateSelectedNotePitch(int p){
    for ( int idx : selectedNotes_ ){
        NoteWidget* n = notes_[idx];
        if ( !n ) continue ;

        n->setMidiNote(n->getMidiNote() + p);
        
        CollectionRequest req ;
        req.collectionType = CollectionType::SEQUENCER ; 
        req.action = CollectionAction::SET ;
        req.index = idx ;
        req.componentId = id_ ;
        req.value = n->getNote() ;

        QJsonObject obj = Util::nlohmannToQJsonObject(req);
        ApiClient::instance()->sendMessage(obj); 
    }
}

void PianoRollWidget::updateSelectedNoteStart(float t){
    for ( int idx : selectedNotes_ ){
        NoteWidget* n = notes_[idx];
        if ( !n ) continue ;
    
        n->setBeatRange(n->getStartBeat() + t, n->getEndBeat() + t);
        CollectionRequest req ;
        req.collectionType = CollectionType::SEQUENCER ; 
        req.action = CollectionAction::SET ;
        req.index = idx ;
        req.componentId = id_ ;
        req.value = n->getNote() ;

        QJsonObject obj = Util::nlohmannToQJsonObject(req);
        ApiClient::instance()->sendMessage(obj); 
    }
}

void PianoRollWidget::updateSelectedNoteDuration(float d){
    for ( int idx : selectedNotes_ ){
        NoteWidget* n = notes_[idx];
        if ( !n ) continue ;
        
        n->setEndBeat(n->getEndBeat() + d);
        CollectionRequest req ;
        req.collectionType = CollectionType::SEQUENCER ; 
        req.action = CollectionAction::SET ;
        req.index = idx ;
        req.componentId = id_ ;
        req.value = n->getNote() ;

        QJsonObject obj = Util::nlohmannToQJsonObject(req);
        ApiClient::instance()->sendMessage(obj); 
    }
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

void PianoRollWidget::onParameterChanged(int componentId, ComponentDescriptor descriptor, ParameterType p, ParameterValue value){
    qDebug() << "Component Parameter Changed: " << componentId << "Parameter:" << GET_PARAMETER_TRAIT_MEMBER(p, name) << ", Value:" << std::get<double>(value) ;
    if ( componentId != id_ ) return ;

    if ( p == ParameterType::DURATION ){
        setTotalBeats(std::get<double>(value));
        return ;
    }


}