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

#include "dsp/AnalyticsEngine.hpp"
#include "core/Engine.hpp"
#include "config/Config.hpp"
#include <cmath>
#include <iostream>
#include <cstring>
#include "kissfft/kiss_fft.h"

AnalyticsEngine* AnalyticsEngine::instance(){
    static AnalyticsEngine* s_instance = nullptr ;
    if ( !s_instance ){
        s_instance = new AnalyticsEngine();
    }
    return s_instance ;
}

AnalyticsEngine::AnalyticsEngine():
    udpSocket_(INVALID_SOCKET),
    bufferPosition_(0)
#ifdef _WIN32
    , wsaInitialized_(false)
#endif
{
    Config::load();
    sampleRate_ = Config::get<unsigned int>("audio.sample_rate").value_or(44100);
    fftSize_ = Config::get<unsigned int>("analysis.buffer_size").value_or(1024);

    fftBuffer_.resize(fftSize_);

    fftConfig_ = kiss_fft_alloc(fftSize_, 0, nullptr, nullptr);
    if ( !fftConfig_ ){
        std::cerr << "Failed to allocate KissFFT config" << std::endl ;
    }
}

AnalyticsEngine::~AnalyticsEngine(){
    stop();

    if ( fftConfig_ ){
        kiss_fft_free(fftConfig_);
        fftConfig_ = nullptr ;
    }
}

void AnalyticsEngine::start(){
    initSocket();
    bufferPosition_ = 0 ;

    std::cout << "AnalyticsEngine started on UDP port " << udpSocket_ << std::endl ;
}

void AnalyticsEngine::stop(){
    closeSocket();
}

void AnalyticsEngine::initSocket() {
#ifdef _WIN32
    if ( !wsaInitialized_ ) {
        WSADATA wsaData;
        if ( WSAStartup(MAKEWORD(2, 2), &wsaData ) != 0 ) {
            std::cerr << "WSAStartup failed" << std::endl ;
            return ;
        }
        wsaInitialized_ = true ;
    }
#endif
    
    udpSocket_ = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if ( udpSocket_ == INVALID_SOCKET ) {
        std::cerr << "UDP socket creation failed" << std::endl ;
        return ;
    }
    
    // Set up destination address (localhost)
    memset(&destAddr_, 0, sizeof(destAddr_));
    destAddr_.sin_family = AF_INET ;
    destAddr_.sin_port = htons(Config::get<unsigned int>("analysis.port").value());
    
#ifdef _WIN32
    destAddr_.sin_addr.S_un.S_addr = inet_addr("127.0.0.1");
#else
    inet_pton(AF_INET, "127.0.0.1", &destAddr_.sin_addr);
#endif
}

void AnalyticsEngine::closeSocket() {
    if (udpSocket_ != INVALID_SOCKET) {
#ifdef _WIN32
        closesocket(udpSocket_);
#else
        close(udpSocket_);
#endif
        udpSocket_ = INVALID_SOCKET ;
    }
    
#ifdef _WIN32
    if ( wsaInitialized_ ) {
        WSACleanup();
        wsaInitialized_ = false ;
    }
#endif
}

void AnalyticsEngine::analyzeBuffer(const double* data, size_t count) {
    if ( count == 0 ) return ;
    
    for ( size_t i = 0; i < count; ++i ) {
        fftBuffer_[bufferPosition_++] = data[i] ;
    
        if ( bufferPosition_ >= fftSize_ ) {
            processFFT();
            
            // Overlap half of buffer for smoother updates
            std::memmove(fftBuffer_.data(), 
                        fftBuffer_.data() + fftSize_ / 2,
                        (fftSize_ / 2) * sizeof(double));
            bufferPosition_ = fftSize_ / 2 ;
        }
    }
}

void AnalyticsEngine::applyHannWindow(std::vector<double>& data) {
    size_t N = data.size() ;
    for ( size_t i = 0; i < N; ++i ) {
        double window = 0.5 * (1.0 - cos(2.0 * M_PI * i / (N - 1)));
        data[i] *= window ;
    }
}

void AnalyticsEngine::processFFT() {
    if ( !fftConfig_ ) return ;

    std::vector<double> windowedData = fftBuffer_ ; // makes a copy
    applyHannWindow(windowedData);
    
    std::vector<kiss_fft_cpx> fftInput(fftSize_);
    std::vector<kiss_fft_cpx> fftOutput(fftSize_);
    
    for ( size_t i = 0; i < fftSize_ ; ++i ) {
        fftInput[i].r = fftBuffer_[i] ;
        fftInput[i].i = 0.0 ;
    }
    
    kiss_fft(fftConfig_, fftInput.data(), fftOutput.data());
    
    // Calculate magnitudes (half size because 2nd half is mirrored)
    std::vector<float> magnitudes(fftSize_ / 2);
    for (size_t i = 0; i < fftSize_ / 2; ++i) {
        kiss_fft_scalar real = fftOutput[i].r ;
        kiss_fft_scalar imag = fftOutput[i].i ;
        float magnitude = std::sqrt(real * real + imag * imag);
        
        magnitude = std::max(magnitude, 1e-10f); // convert to db, floor for log(0)
        magnitudes[i] = 20.0 * std::log10(magnitude);
    }
    
    sendFFTData(magnitudes);
}

void AnalyticsEngine::sendFFTData(const std::vector<float>& magnitudes) {
    if ( udpSocket_ == INVALID_SOCKET ) return ;
    
    // Simple binary format: [float, float, float, ...]
    const char* data = reinterpret_cast<const char*>(magnitudes.data());
    size_t dataSize = magnitudes.size() * sizeof(float);
    
    sendto(udpSocket_, data, static_cast<int>(dataSize), 0,
        (struct sockaddr*)&destAddr_, sizeof(destAddr_));
    
}