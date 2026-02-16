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

#include "widgets/SpectrumAnalyzerWidget.hpp"
#include "app/Theme.hpp"
#include "config/Config.hpp"
#include <QPaintEvent>
#include <QPainterPath>
#include <cmath>


SpectrumAnalyzerWidget::SpectrumAnalyzerWidget(QWidget *parent):
    QWidget(parent),
    udpSocket_(new QUdpSocket(this)),
    minFreq_(10.0),
    maxFreq_(25000.0),
    minDb_(-100.0),
    maxDb_(5.0),
    updateTimer_(new QTimer(this)),
    dataReady_(false)
{
    Config::load();

    port_ = Config::get<unsigned int>("analysis.spectrum_analyzer.port").value_or(54322);
    sampleRate_ = Config::get<float>("audio.sample_rate").value_or(44100);
    fftSize_ = Config::get<unsigned int>("analysis.spectrum_analyzer.buffer_size").value_or(2048);
    smoothFactor_ = Config::get<float>("analysis.spectrum_analyzer.smooth_factor").value_or(0.7);

    spectrumData_.resize(fftSize_ / 2, minDb_);
    smoothedData_.resize(fftSize_ / 2, minDb_);
    updateTimer_->setInterval(33); // ~30 FPS

    setMinimumSize(400, 300);
    setWindowTitle("Spectrum Analyzer");

    connect(udpSocket_, &QUdpSocket::readyRead, this, &SpectrumAnalyzerWidget::onReadyRead);
    connect(updateTimer_, &QTimer::timeout, this, &SpectrumAnalyzerWidget::onUpdateTimeout);
    
    updateTimer_->start();
    
}

SpectrumAnalyzerWidget::~SpectrumAnalyzerWidget() {
    if (udpSocket_->state() == QAbstractSocket::BoundState) {
        udpSocket_->close();
    }
}

void SpectrumAnalyzerWidget::setPort(quint16 port) {
    if (udpSocket_->state() == QAbstractSocket::BoundState) {
        udpSocket_->close();
    }
    
    port_ = port;
    
    if (!udpSocket_->bind(QHostAddress::LocalHost, port_)) {
        qWarning() << "Failed to bind UDP socket to port" << port_;
    } else {
        qDebug() << "Spectrum analyzer listening on UDP port" << port_;
    }
}

void SpectrumAnalyzerWidget::setFrequencyRange(float minHz, float maxHz) {
    minFreq_ = minHz;
    maxFreq_ = maxHz;
    update();
}

void SpectrumAnalyzerWidget::setMagnitudeRange(float minDb, float maxDb) {
    minDb_ = minDb;
    maxDb_ = maxDb;
    update();
}

void SpectrumAnalyzerWidget::setSampleRate(float sampleRate) {
    sampleRate_ = sampleRate;
    update();
}

void SpectrumAnalyzerWidget::onReadyRead() {
    while (udpSocket_->hasPendingDatagrams()) {
        QByteArray datagram ;
        datagram.resize(udpSocket_->pendingDatagramSize());
        udpSocket_->readDatagram(datagram.data(), datagram.size());
        
        // Parse float array
        const float* data = reinterpret_cast<const float*>(datagram.data());
        size_t count = datagram.size() / sizeof(float) ;
        
        if ( count > 0 ){
            spectrumData_.assign(data, data + count);

            if ( smoothedData_.size() != count ){ // don't smooth on first packet
                smoothedData_.resize(count);
                smoothedData_ = spectrumData_ ; 
            } else {
                for ( size_t i = 0 ; i < count; ++i ){
                    smoothedData_[i] = smoothFactor_ * smoothedData_[i] + 
                        (1.0f - smoothFactor_) * spectrumData_[i] ;
                }
            }

            fftSize_ = count * 2 ; 
            dataReady_ = true ;
        }
    }
}

void SpectrumAnalyzerWidget::onUpdateTimeout() {
    if ( dataReady_ ) {
        renderToCache() ;
        update(); 
        dataReady_ = false ;
    }
}

void SpectrumAnalyzerWidget::paintEvent(QPaintEvent *event) {
    Q_UNUSED(event);
    
    QPainter painter(this);

    if ( !cachedFrame_.isNull() ){
        painter.drawImage(0,0, cachedFrame_);
    }
}

void SpectrumAnalyzerWidget::resizeEvent(QResizeEvent *event) {
    Q_UNUSED(event);
    cachedFrame_ = QImage();
    update();
}

void SpectrumAnalyzerWidget::drawGrid(QPainter &painter) {
    painter.setPen(Theme::ANALYZER_GRID_COLOR);
    
    int plotWidth = width() - Theme::ANALYZER_MARGIN_LEFT - Theme::ANALYZER_MARGIN_RIGHT ;
    int plotHeight = height() - Theme::ANALYZER_MARGIN_TOP - Theme::ANALYZER_MARGIN_BOTTOM ;
    
    // Horizontal grid lines (dB)
    for (float db = minDb_; db <= maxDb_; db += 10.0) {
        int y = static_cast<int>(dbToY(db));
        painter.drawLine(Theme::ANALYZER_MARGIN_LEFT, y, Theme::ANALYZER_MARGIN_LEFT + plotWidth, y);
    }
    
    // Vertical grid lines (frequency, logarithmic)
    std::vector<float> freqs = {20, 50, 100, 2.0, 500, 1000, 2000, 5000, 10000, 20000};
    for (float freq : freqs) {
        if (freq >= minFreq_ && freq <= maxFreq_) {
            int x = static_cast<int>(freqToX(freq));
            painter.drawLine(x, Theme::ANALYZER_MARGIN_TOP, x, Theme::ANALYZER_MARGIN_TOP + plotHeight);
        }
    }
}

void SpectrumAnalyzerWidget::drawSpectrum(QPainter &painter) {
    if (smoothedData_.empty()) return;
    
    // Create path for spectrum
    QPainterPath path ;
    bool firstPoint = true ;
    
    int plotWidth = width() - Theme::ANALYZER_MARGIN_LEFT - Theme::ANALYZER_MARGIN_RIGHT ;

    int sampleInterval = Theme::ANALYZER_PIXEL_RESOLUTION ;
    for ( int px = 0 ; px < plotWidth; px += sampleInterval ){
        float x = Theme::ANALYZER_MARGIN_LEFT + px ;
        float freq = xToFreq(x);
        
        if ( freq < minFreq_ || freq > maxFreq_ ) continue ;

        size_t bin = freqToBin(freq);
        if ( bin >= smoothedData_.size() ) continue ;

        float db = std::max(minDb_, std::min(maxDb_, smoothedData_[bin]));
        float y = dbToY(db);

        if ( firstPoint ){
            path.moveTo(x,y);
            firstPoint = false ;
        } else {
            path.lineTo(x,y);
        }
    }
    
    // Draw the spectrum line
    painter.setPen(QPen(Theme::ANALYZER_LINE_COLOR, 2));
    painter.drawPath(path);
}

void SpectrumAnalyzerWidget::drawLabels(QPainter &painter) {
    painter.setPen(Theme::COMPONENT_TEXT);
    QFont font = painter.font();
    font.setPointSize(9);
    painter.setFont(font);
    
    int plotWidth = width() - Theme::ANALYZER_MARGIN_LEFT - Theme::ANALYZER_MARGIN_RIGHT;
    int plotHeight = height() - Theme::ANALYZER_MARGIN_TOP - Theme::ANALYZER_MARGIN_BOTTOM;
    
    // Y-axis labels (dB)
    for (float db = minDb_; db <= maxDb_; db += 20.0) {
        int y = static_cast<int>(dbToY(db));
        QString label = QString::number(static_cast<int>(db)) + " dB";
        painter.drawText(5, y + 5, label);
    }
    
    // X-axis labels (frequency)
    std::vector<std::pair<float, QString>> freqLabels = {
        {20, "20Hz"},
        {50, "50"},
        {100, "100"},
        {200, "200"},
        {500, "500"},
        {1000, "1kHz"},
        {2000, "2k"},
        {5000, "5k"},
        {10000, "10k"},
        {20000, "20k"}
    };
    
    for (const auto& [freq, label] : freqLabels) {
        if (freq >= minFreq_ && freq <= maxFreq_) {
            int x = static_cast<int>(freqToX(freq));
            painter.drawText(x - 15, height() - 5, label);
        }
    }
}

void SpectrumAnalyzerWidget::renderToCache() {
    if ( cachedFrame_.size() != size()){
        cachedFrame_ = QImage(size(), QImage::Format_ARGB32_Premultiplied);
    }

    cachedFrame_.fill(Theme::ANALYZER_BACKGROUND_COLOR);
    
    QPainter painter(&cachedFrame_);
    painter.setRenderHint(QPainter::Antialiasing);

    drawGrid(painter);
    drawSpectrum(painter);
    drawLabels(painter);

}

float SpectrumAnalyzerWidget::freqToX(float freq) const {
    int plotWidth = width() - Theme::ANALYZER_MARGIN_LEFT - Theme::ANALYZER_MARGIN_RIGHT ;
    
    // Logarithmic mapping
    float logMin = std::log10(minFreq_);
    float logMax = std::log10(maxFreq_);
    float logFreq = std::log10(freq);
    
    float normalized = (logFreq - logMin) / (logMax - logMin);
    return Theme::ANALYZER_MARGIN_LEFT + normalized * plotWidth;
}

float SpectrumAnalyzerWidget::xToFreq(float x) const {    
    int plotWidth = width() - Theme::ANALYZER_MARGIN_LEFT - Theme::ANALYZER_MARGIN_RIGHT ;
    float normalized = (x - Theme::ANALYZER_MARGIN_LEFT) / plotWidth ;

    float logMin = std::log10(minFreq_);
    float logMax = std::log10(maxFreq_);
    float logFreq = logMin + normalized * (logMax - logMin);

    return std::pow(10.0, logFreq);
}

float SpectrumAnalyzerWidget::dbToY(float db) const {
    int plotHeight = height() - Theme::ANALYZER_MARGIN_TOP - Theme::ANALYZER_MARGIN_BOTTOM;
    
    // Linear mapping (inverted - lower dB = higher on screen)
    float normalized = (db - minDb_) / (maxDb_ - minDb_) ;
    return Theme::ANALYZER_MARGIN_TOP + (1.0 - normalized) * plotHeight ;
}

float SpectrumAnalyzerWidget::binToFreq(size_t bin) const {
    return (bin * sampleRate_) / fftSize_ ;
}

size_t SpectrumAnalyzerWidget::freqToBin(float freq) const {
    return static_cast<size_t>((freq * fftSize_) / sampleRate_ );
}