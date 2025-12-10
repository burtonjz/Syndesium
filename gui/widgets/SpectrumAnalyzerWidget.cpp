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
#include "config/Config.hpp"
#include <QPaintEvent>
#include <QPainterPath>
#include <cmath>


SpectrumAnalyzerWidget::SpectrumAnalyzerWidget(QWidget *parent):
    QWidget(parent),
    udpSocket_(new QUdpSocket(this)),
    minFreq_(20.0),
    maxFreq_(20000.0),
    minDb_(-100.0),
    maxDb_(60.0),
    updateTimer_(new QTimer(this)),
    dataReady_(false)
{
    Config::load();

    port_ = Config::get<unsigned int>("analysis.port").value_or(54322);
    sampleRate_ = Config::get<double>("audio.sample_rate").value_or(44100);
    fftSize_ = Config::get<unsigned int>("analysis.buffer_size").value_or(2048);
    
    spectrumData_.resize(fftSize_ / 2, minDb_);
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

void SpectrumAnalyzerWidget::setFrequencyRange(double minHz, double maxHz) {
    minFreq_ = minHz;
    maxFreq_ = maxHz;
    update();
}

void SpectrumAnalyzerWidget::setMagnitudeRange(double minDb, double maxDb) {
    minDb_ = minDb;
    maxDb_ = maxDb;
    update();
}

void SpectrumAnalyzerWidget::setSampleRate(double sampleRate) {
    sampleRate_ = sampleRate;
    update();
}

void SpectrumAnalyzerWidget::onReadyRead() {
    while (udpSocket_->hasPendingDatagrams()) {
        QByteArray datagram;
        datagram.resize(udpSocket_->pendingDatagramSize());
        udpSocket_->readDatagram(datagram.data(), datagram.size());
        
        // Parse float array
        const float* data = reinterpret_cast<const float*>(datagram.data());
        size_t count = datagram.size() / sizeof(float);
        
        if (count > 0) {
            spectrumData_.assign(data, data + count);
            fftSize_ = count * 2; // Assuming we receive half the FFT (Nyquist)
            dataReady_ = true;
        }
    }
}

void SpectrumAnalyzerWidget::onUpdateTimeout() {
    if (dataReady_) {
        update(); // Trigger repaint
        dataReady_ = false;
    }
}

void SpectrumAnalyzerWidget::paintEvent(QPaintEvent *event) {
    Q_UNUSED(event);
    
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    
    // Background
    painter.fillRect(rect(), QColor(20, 20, 20));
    
    // Draw components
    drawGrid(painter);
    drawSpectrum(painter);
    drawLabels(painter);
}

void SpectrumAnalyzerWidget::resizeEvent(QResizeEvent *event) {
    Q_UNUSED(event);
    update();
}

void SpectrumAnalyzerWidget::drawGrid(QPainter &painter) {
    painter.setPen(QColor(60, 60, 60));
    
    int plotWidth = width() - MARGIN_LEFT - MARGIN_RIGHT;
    int plotHeight = height() - MARGIN_TOP - MARGIN_BOTTOM;
    
    // Horizontal grid lines (dB)
    for (double db = minDb_; db <= maxDb_; db += 10.0) {
        int y = static_cast<int>(dbToY(db));
        painter.drawLine(MARGIN_LEFT, y, MARGIN_LEFT + plotWidth, y);
    }
    
    // Vertical grid lines (frequency, logarithmic)
    std::vector<double> freqs = {20, 50, 100, 200, 500, 1000, 2000, 5000, 10000, 20000};
    for (double freq : freqs) {
        if (freq >= minFreq_ && freq <= maxFreq_) {
            int x = static_cast<int>(freqToX(freq));
            painter.drawLine(x, MARGIN_TOP, x, MARGIN_TOP + plotHeight);
        }
    }
}

void SpectrumAnalyzerWidget::drawSpectrum(QPainter &painter) {
    if (spectrumData_.empty()) return;
    
    // Create path for spectrum
    QPainterPath path;
    bool firstPoint = true;
    
    for (size_t bin = 0; bin < spectrumData_.size(); ++bin) {
        double freq = binToFreq(bin);
        
        // Only draw frequencies in our display range
        if (freq < minFreq_ || freq > maxFreq_) continue;
        
        double db = spectrumData_[bin];
        db = std::max(minDb_, std::min(maxDb_, db)); // Clamp
        
        double x = freqToX(freq);
        double y = dbToY(db);
        
        if (firstPoint) {
            path.moveTo(x, y);
            firstPoint = false;
        } else {
            path.lineTo(x, y);
        }
    }
    
    // Draw the spectrum line
    painter.setPen(QPen(QColor(0, 255, 128), 2));
    painter.drawPath(path);
}

void SpectrumAnalyzerWidget::drawLabels(QPainter &painter) {
    painter.setPen(QColor(200, 200, 200));
    QFont font = painter.font();
    font.setPointSize(9);
    painter.setFont(font);
    
    int plotWidth = width() - MARGIN_LEFT - MARGIN_RIGHT;
    int plotHeight = height() - MARGIN_TOP - MARGIN_BOTTOM;
    
    // Y-axis labels (dB)
    for (double db = minDb_; db <= maxDb_; db += 20.0) {
        int y = static_cast<int>(dbToY(db));
        QString label = QString::number(static_cast<int>(db)) + " dB";
        painter.drawText(5, y + 5, label);
    }
    
    // X-axis labels (frequency)
    std::vector<std::pair<double, QString>> freqLabels = {
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

double SpectrumAnalyzerWidget::freqToX(double freq) const {
    int plotWidth = width() - MARGIN_LEFT - MARGIN_RIGHT;
    
    // Logarithmic mapping
    double logMin = std::log10(minFreq_);
    double logMax = std::log10(maxFreq_);
    double logFreq = std::log10(freq);
    
    double normalized = (logFreq - logMin) / (logMax - logMin);
    return MARGIN_LEFT + normalized * plotWidth;
}

double SpectrumAnalyzerWidget::dbToY(double db) const {
    int plotHeight = height() - MARGIN_TOP - MARGIN_BOTTOM;
    
    // Linear mapping (inverted - lower dB = higher on screen)
    double normalized = (db - minDb_) / (maxDb_ - minDb_);
    return MARGIN_TOP + (1.0 - normalized) * plotHeight;
}

double SpectrumAnalyzerWidget::binToFreq(size_t bin) const {
    return (bin * sampleRate_) / fftSize_;
}