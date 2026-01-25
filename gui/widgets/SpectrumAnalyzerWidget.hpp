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

private:
    // UDP socket for receiving FFT data
    QUdpSocket *udpSocket_;
    quint16 port_;
    
    // FFT data
    std::vector<float> spectrumData_ ;
    std::vector<float> smoothedData_ ;

    size_t fftSize_;
    float smoothFactor_ ;
    float sampleRate_;
    
    // Display ranges
    float minFreq_;
    float maxFreq_;
    float minDb_;
    float maxDb_;
    
    // Update throttling
    QTimer *updateTimer_;
    bool dataReady_;

    QImage cachedFrame_ ;

public:
    explicit SpectrumAnalyzerWidget(QWidget *parent = nullptr);
    ~SpectrumAnalyzerWidget() override;

    void setPort(quint16 port);
    void setFrequencyRange(float minHz, float maxHz);
    void setMagnitudeRange(float minDb, float maxDb);
    void setSampleRate(float sampleRate);

protected:
    void paintEvent(QPaintEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;

private slots:
    void onReadyRead();
    void onUpdateTimeout();

private:
    // Helper methods
    void drawGrid(QPainter &painter);
    void drawSpectrum(QPainter &painter);
    void drawLabels(QPainter &painter);
    void renderToCache();

    // Coordinate conversion
    float freqToX(float freq) const ;
    float xToFreq(float x) const ;
    float dbToY(float db) const ;
    float binToFreq(size_t bin) const ;
    size_t freqToBin(float freq) const ;
    
};

#endif // SPECTRUM_ANALYZER_WIDGET_HPP