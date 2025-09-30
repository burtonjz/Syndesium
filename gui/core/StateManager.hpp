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

#ifndef STATEMANAGER_HPP
#define STATEMANAGER_HPP

#include <QObject>

class StateManager : public QObject
{
    Q_OBJECT

private:
    bool setupAudio_ = false ;
    bool setupMidi_  = false ;
    bool running_    = false ;

public:
    explicit StateManager(QObject *parent = nullptr);

    void setSetupAudioComplete(bool v);
    void setSetupMidiComplete(bool v);
    bool isRunning() const ;
    void setRunning(bool v);

private:
    void checkSetupConditions();

signals:
    void setupCompleted();

};

#endif // STATEMANAGER_HPP
