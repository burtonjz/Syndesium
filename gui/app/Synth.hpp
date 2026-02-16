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

#ifndef __UI_SYNTH_HPP_
#define __UI_SYNTH_HPP_

#include <QMainWindow>
#include <QUiLoader>
#include <QFile>
#include <QWidget>
#include <QVBoxLayout>
#include <QPushButton>
#include <qevent.h>
#include <qtmetamacros.h>

#include "app/ModuleContext.hpp"
#include "views/GraphPanel.hpp"
#include "app/Setup.hpp"
#include "widgets/SpectrumAnalyzerWidget.hpp"

#include "ui_Synth.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class Synth : public QMainWindow {
    Q_OBJECT

private:
    Ui::MainWindow* ui_ ;
    ModuleContext ctx_ ;
    GraphPanel* graph_ ;
    Setup* setup_ ;

    // menu widgets
    QPointer<SpectrumAnalyzerWidget> spectrumWidget_ ;

    // save/load 
    QString saveFilePath_ ;
    QJsonObject saveData_ ;
    bool hasUnsavedChanges_ ;

public:
    Synth(ModuleContext ctx, QWidget* parent = nullptr);
    ~Synth();

private:
    void configureMenuActions();
    void configureWidgetButtons();

    void performSave();

    void closeEvent(QCloseEvent* event) override ;

signals:
    void engineStatusChanged(bool status);
    void componentAdded(ComponentType typ);

private slots:
    void onApiConnected();
    void onApiDataReceived(const QJsonObject &json);
    void onSetupButtonClicked();
    void onStartStopButtonClicked();
    void onEngineStatusChange(bool status);
    void onComponentAdded(int index);

    // menu bar actions
    void onActionLoad();
    void onActionSave();
    void onActionSaveAs();
    void onActionSpectrumAnalyzer();

public slots:
    void markModified();

};

#endif // __UI_SYNTH_HPP_