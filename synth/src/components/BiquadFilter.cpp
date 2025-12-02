#include "BiquadFilter.hpp"
#include "config/Config.hpp"
#include "core/BaseComponent.hpp"
#include "params/ParameterMap.hpp"
#include "types/FilterType.hpp"
#include "types/ParameterType.hpp"
#include <cmath>

BiquadFilter::BiquadFilter(ComponentId id, BiquadFilterConfig cfg):
    BaseComponent(id, ComponentType::BiquadFilter),
    BaseModule(),
    BaseModulator()
{
    parameters_->add<ParameterType::FILTER_TYPE>(cfg.filterType, false);
    parameters_->add<ParameterType::FREQUENCY>(cfg.frequency, true);
    parameters_->add<ParameterType::DBGAIN>(cfg.gain, true);
    parameters_->add<ParameterType::Q_FACTOR>(cfg.qFactor,true);
    parameters_->add<ParameterType::BANDWIDTH>(cfg.bandwidth,true);
    parameters_->add<ParameterType::SHELF>(cfg.shelfSlope,true);

    sampleRate_ = Config::get<double>("audio.sample_rate").value();
}

// see https://webaudio.github.io/Audio-EQ-Cookbook/audio-eq-cookbook.html
void BiquadFilter::calculateCoefficients(){
    double b0, b1, b2, a0, a1, a2 ;
    double w0, alpha, sin_w0, cos_w0, A, S, Q ;

    w0 = 2.0 * M_PI * parameters_->getParameter<ParameterType::FREQUENCY>()->getInstantaneousValue() / sampleRate_ ;
    cos_w0 = std::cos(w0);
    sin_w0 = std::sin(w0);

    switch(FilterType::from_uint8(parameters_->getParameter<ParameterType::FILTER_TYPE>()->getValue())){
    case FilterType::LowPass:
        Q = parameters_->getParameter<ParameterType::Q_FACTOR>()->getInstantaneousValue();
        alpha = sin_w0 / ( 2.0 * Q );
        b0 = (1.0 - cos_w0) / 2.0 ; // b0
        b1 = 1.0 - cos_w0 ; // b1
        b2 = b0 ; // b2
        a0 = 1.0 + alpha ; // a0
        a1 = -2.0 * cos_w0 ; // a1
        a2 = 1.0 - alpha ; // a2
        break ;
    case FilterType::HighPass:
        Q = parameters_->getParameter<ParameterType::Q_FACTOR>()->getInstantaneousValue();
        alpha = sin_w0 / ( 2.0 * Q);
        b0 = (1.0 + cos_w0) / 2.0 ;
        b1 = -1.0 - cos_w0 ;
        b2 = b0 ;
        a0 = 1.0 + alpha ;
        a1 = -2.0 * cos_w0 ;
        a2 = 1.0 - alpha ;
        break ;
    case FilterType::BandPass:
        Q = parameters_->getParameter<ParameterType::Q_FACTOR>()->getInstantaneousValue();
        alpha = sin_w0 / ( 2.0 * Q);
        b0 = Q * alpha ;
        b1 = 0.0 ;
        b2 = -b0 ;
        a0 = 1.0 + alpha ;
        a1 = -2.0 * cos_w0 ;
        a2 = 1.0 - alpha ;
        break ;
    case FilterType::LowShelf:
        S = parameters_->getParameter<ParameterType::SHELF>()->getInstantaneousValue() ;
        A = std::pow(10.0, parameters_->getParameter<ParameterType::DBGAIN>()->getInstantaneousValue() / 40.0 );
        alpha = (sin_w0 / 2.0) * std::sqrt((A + 1.0 / A) * ( 1.0 / S - 1.0 ) + 2.0 );
        b0 = A * ((A + 1.0) - (A - 1.0) * cos_w0 + 2.0 * std::sqrt(A) * alpha );
        b1 = 2.0 * A * ((A - 1.0) - (A + 1.0) * cos_w0);
        b2 = A * ((A + 1.0) - (A - 1.0) * cos_w0 - 2.0 * std::sqrt(A) * alpha );
        a0 = (A + 1.0) + (A - 1.0) * cos_w0 + 2.0 * std::sqrt(A) * alpha ;
        a1 = -2.0 * ((A - 1.0) + (A + 1.0) * cos_w0 );
        a2 = (A + 1.0) + (A - 1.0) * cos_w0 - 2.0 * std::sqrt(A) * alpha ;
        break ;
    case FilterType::HighShelf:
        S = parameters_->getParameter<ParameterType::SHELF>()->getInstantaneousValue() ;
        A = std::pow(10.0, parameters_->getParameter<ParameterType::DBGAIN>()->getInstantaneousValue() / 40.0 );
        alpha = (sin_w0 / 2.0) * std::sqrt((A + 1.0 / A) * ( 1.0 / S - 1.0 ) + 2.0 );
        b0 = A * ((A + 1.0) + (A - 1.0) * cos_w0 + 2.0 * std::sqrt(A) * alpha );
        b1 = -2.0 * A * ((A - 1.0) + (A + 1.0) * cos_w0);
        b2 = A * ((A + 1.0) + (A - 1.0) * cos_w0 - 2.0 * std::sqrt(A) * alpha );
        a0 = (A + 1.0) - (A - 1.0) * cos_w0 + 2.0 * std::sqrt(A) * alpha ;
        a1 = 2.0 * ((A - 1.0) - (A + 1.0) * cos_w0 );
        a2 = (A + 1.0) - (A - 1.0) * cos_w0 - 2.0 * std::sqrt(A) * alpha ;
        break ;
    default:
        throw std::runtime_error("unknown filter type configured");
    }

    // normalize coefficients:
    coefficients_ = {b0/a0, b1/a0, b2/a0, a1/a0, a2/a0};
}

inline double BiquadFilter::getCurrentOutput(double x0, double x1, double x2, double y1, double y2) const {
    return (coefficients_[0])* x0 + 
        (coefficients_[1]) * x1  + 
        (coefficients_[2]) * x2  - 
        (coefficients_[3]) * y1 - 
        (coefficients_[4]) * y2 ;
    
}

double BiquadFilter::modulate(double value, ModulationData* mData) const {
    double output = 0 ;

    // check required data
    if ( !mData ) return output ; 
    if ( ! mData->has(ModulationParameter::INPUT_1) ){
        mData->set(ModulationParameter::INPUT_1, 0.0);
    }
    if ( ! mData->has(ModulationParameter::INPUT_2) ){
        mData->set(ModulationParameter::INPUT_2, 0.0);
    }
    if ( ! mData->has(ModulationParameter::OUTPUT_1) ){
        mData->set(ModulationParameter::OUTPUT_1, 0.0);
    }
    if ( ! mData->has(ModulationParameter::OUTPUT_2) ){
        mData->set(ModulationParameter::OUTPUT_2, 0.0);
    }

    output = getCurrentOutput(
        value,
        mData->get(ModulationParameter::INPUT_1),
        mData->get(ModulationParameter::INPUT_2),
        mData->get(ModulationParameter::OUTPUT_1),
        mData->get(ModulationParameter::OUTPUT_2)
    );

    // tick modulation data
    mData->set(ModulationParameter::OUTPUT_2, mData->get(ModulationParameter::OUTPUT_1));
    mData->set(ModulationParameter::OUTPUT_1, output);
    mData->set(ModulationParameter::INPUT_2, mData->get(ModulationParameter::INPUT_1));
    mData->set(ModulationParameter::INPUT_1, value);
    
    return output ;
}

void BiquadFilter::calculateSample(){
    double input = 0, output = 0 ;

    for (auto m : getInputs()){
        input += m->getCurrentSample();
    }

    output = getCurrentOutput(input, lastInputs_[0], lastInputs_[1], lastOutputs_[0], lastOutputs_[1]);
    buffer_[bufferIndex_] = output ;

    // tick inputs/outputs
    lastOutputs_[1] = lastOutputs_[0];
    lastOutputs_[0] = output ;
    lastInputs_[1] = lastInputs_[0];
    lastInputs_[0] = input ;

}

void BiquadFilter::tick(){
    BaseModule::tick();
    calculateCoefficients();
}
