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


#ifndef SPECTRUM_ANALYZER_WIDGET_HPP
#define SPECTRUM_ANALYZER_WIDGET_HPP

#include <QWidget>
#include <QUdpSocket>
#include <vector>
#include <QPainter>
#include <QTimer>

class SpectrumAnalyzerWidget : public QWidget {
    Q_OBJECT

public:
    explicit SpectrumAnalyzerWidget(QWidget *parent = nullptr);
    ~SpectrumAnalyzerWidget() override;

    void setPort(quint16 port);
    void setFrequencyRange(double minHz, double maxHz);
    void setMagnitudeRange(double minDb, double maxDb);
    void setSampleRate(double sampleRate);

protected:
    void paintEvent(QPaintEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;

private slots:
    void onReadyRead();
    void onUpdateTimeout();

private:
    // UDP socket for receiving FFT data
    QUdpSocket *udpSocket_;
    quint16 port_;
    
    // FFT data
    std::vector<float> spectrumData_ ;
    std::vector<float> smoothedData_ ;
    
    size_t fftSize_;
    float smoothFactor_ ;
    double sampleRate_;
    
    // Display ranges
    double minFreq_;
    double maxFreq_;
    double minDb_;
    double maxDb_;
    
    // Update throttling
    QTimer *updateTimer_;
    bool dataReady_;
    
    // Helper methods
    void drawGrid(QPainter &painter);
    void drawSpectrum(QPainter &painter);
    void drawLabels(QPainter &painter);
    
    // Coordinate conversion
    double freqToX(double freq) const;
    double dbToY(double db) const;
    double binToFreq(size_t bin) const;
    
};

#endif // SPECTRUM_ANALYZER_WIDGET_HPP