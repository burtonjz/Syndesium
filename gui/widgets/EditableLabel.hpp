/*
 * Copyright (C) 2026 Jared Burton
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

#ifndef EDITABLE_LABEL_HPP_
#define EDITABLE_LABEL_HPP_

#include <QLabel>
#include <QLineEdit>
#include <QMouseEvent>

class EditableLabel : public QLabel
{
    Q_OBJECT

private:    
    QLineEdit* lineEdit_ ;

public:
    EditableLabel(const QString& text, QWidget* parent = nullptr): 
        QLabel(text, parent), 
        lineEdit_(nullptr)
    {}

protected:
    void mouseDoubleClickEvent(QMouseEvent* event) override {
        if ( event->button() == Qt::LeftButton ){
            startEditing();
        }
        QLabel::mouseDoubleClickEvent(event);
    }

private slots:
    void finishEditing(){
        if ( lineEdit_ ){
            setText(lineEdit_->text());
            lineEdit_->deleteLater();
            lineEdit_ = nullptr;
            show();
        }
    }

private:
    void startEditing(){
        if ( lineEdit_ ) return;

        lineEdit_ = new QLineEdit(text(), parentWidget());
        lineEdit_->setGeometry(geometry());
        lineEdit_->setFont(font());
        lineEdit_->selectAll();
        lineEdit_->show();
        lineEdit_->setFocus();

        connect(lineEdit_, &QLineEdit::editingFinished, this, &EditableLabel::finishEditing);

        hide();
    }
};

#endif // EDITABLE_LABEL_HPP_