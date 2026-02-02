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

#include "widgets/ParameterWidget.hpp"
#include "config/Config.hpp"
#include "core/Theme.hpp"
#include "types/ParameterType.hpp"
#include "types/Waveform.hpp"
#include "types/FilterType.hpp"
#include "widgets/ComponentDetailWidget.hpp"

#include <cmath>
#include <QVBoxLayout>
#include <QHBoxLayout>

/*
=========================================
================= DELAY =================
=========================================
*/
DelayWidget::DelayWidget(QWidget* parent): 
    ParameterWidget(parent),
    label_(nullptr),
    slider_(nullptr),
    unitCombo_(nullptr),
    valueLabel_(nullptr)
{
    sampleRate_ = Config::get<double>("audio.sample_rate").value();
    setupUI();
    connectSignals();
    updateDisplay();
}

ParameterValue DelayWidget::getValue() const {
    QString unit = unitCombo_->currentText();
    if (unit == "ms") { 
            return static_cast<int>(slider_->value() / 1000.0f * sampleRate_);
    } else {
        return static_cast<int>(slider_->value());
    }
}

void DelayWidget::setValue(const ParameterValue& value){
    using ValueType = GET_PARAMETER_VALUE_TYPE(ParameterType::DELAY);

    size_t samples = std::get<ValueType>(value);

    setValue(samples);
}

void DelayWidget::setValue(size_t samples){
    QString unit = unitCombo_->currentText();
    QSignalBlocker blocker(slider_);

    if (unit == "ms") { 
        slider_->setValue((samples / sampleRate_) * 1000.0f) ;
    } else {
        slider_->setValue(samples);
    }

    updateDisplay();
}

void DelayWidget::setupUI(){
    // default setup is in samples
    // can toggle to time in ms
    QVBoxLayout* layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(Theme::COMPONENT_DETAIL_WIDGET_SPACING);
    
    // widget label
    label_ = new QLabel("Delay Time");
    label_->setAlignment(Qt::AlignCenter);
    layout->addWidget(label_);

    // value label
    valueLabel_ = new QLabel("0 samples");
    valueLabel_->setAlignment(Qt::AlignCenter);
    layout->addWidget(valueLabel_);
    
    // value slider
    slider_ = new QSlider(Qt::Horizontal);
    slider_->setMinimum(0);
    slider_->setMaximum(sampleRate_);  
    slider_->setValue(0);
    slider_->setEnabled(true);
    layout->addWidget(slider_);
    
    // unit selector
    QWidget* unitContainer = new QWidget();
    QHBoxLayout* unitLayout = new QHBoxLayout(unitContainer);
    unitLayout->setContentsMargins(0, 0, 0, 0);
    
    QLabel* unitLabel = new QLabel("Unit:");
    unitCombo_ = new QComboBox();
    unitCombo_->addItem("samples");
    unitCombo_->addItem("ms");
    
    unitLayout->addWidget(unitLabel);
    unitLayout->addWidget(unitCombo_);
    unitLayout->addStretch();
    
    layout->addWidget(unitContainer);
}

void DelayWidget::connectSignals(){
    connect(slider_, &QSlider::valueChanged, this, [this](){
        updateDisplay();
        emit valueChanged();
    });

    auto p = dynamic_cast<ComponentDetailWidget*>(parent());
    if ( p ){
        connect(this, &ParameterWidget::valueChanged, p, &ComponentDetailWidget::onValueChange);
    }

    // changing units updates the slider but doesn't send updates
    connect(unitCombo_, QOverload<int>::of(&QComboBox::currentIndexChanged), 
            this, [this](int index) {
        size_t samples ; 
        QString oldUnit = (index == 0) ? "ms" : "samples"; 
        QString newUnit = unitCombo_->currentText();
        double sampleRate = Config::get<double>("audio.sample_rate").value();

        if (oldUnit == "ms") { 
            samples = slider_->value() / 1000.0f * sampleRate ;
        } else {
            samples = slider_->value();
        }

        // Update slider for new unit
        if (newUnit == "ms") {
            slider_->setMaximum(4000); 
            setValue(samples);
        } else {
            int maxSamples = sampleRate ; 
            slider_->setMaximum(maxSamples);
            slider_->setValue(samples);  
        }
    });
}

void DelayWidget::updateDisplay(){
    QString unit = unitCombo_->currentText();
    int val = slider_->value();

    if (unit == "ms") {
        valueLabel_->setText(QString::number(val, 'f', 1) + " ms");
    } else {
        if ( val == 1 ){
            valueLabel_->setText(QString::number(val) + " sample");
        } else {
            valueLabel_->setText(QString::number(val) + " samples");
        }
        
    }
}

/*
=========================================
================ WAVEFORM ===============
=========================================
*/
WaveformWidget::WaveformWidget(QWidget* parent):
    label_(nullptr),
    waveforms_(nullptr)
{
    QVBoxLayout* layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(Theme::COMPONENT_DETAIL_WIDGET_SPACING);

    // label
    label_ = new QLabel("Waveform");
    layout->addWidget(label_);

    // populate waveforms
    waveforms_ = new QComboBox(this);
    for ( auto wf : Waveform::getWaveforms()){
        Waveform wave = Waveform(wf);
        waveforms_->addItem(QString::fromStdString(wave.toString()),wave.to_uint8());
    }

    // set default waveform
    int idx = waveforms_->findData(GET_PARAMETER_TRAIT_MEMBER(ParameterType::WAVEFORM, defaultValue));
    if ( idx != -1 ){
        waveforms_->setCurrentIndex(idx);
    } 
    layout->addWidget(waveforms_);

    // connections
    connect(waveforms_, &QComboBox::currentIndexChanged, this, &ParameterWidget::valueChanged);

    auto p = dynamic_cast<ComponentDetailWidget*>(parent);
    if ( p ){
        connect(this, &ParameterWidget::valueChanged, p, &ComponentDetailWidget::onValueChange);
    }

}

ParameterValue WaveformWidget::getValue() const {
    return waveforms_->itemData(waveforms_->currentIndex()).value<uint8_t>();
}

void WaveformWidget::setValue(const ParameterValue& value){
    using ValueType = GET_PARAMETER_VALUE_TYPE(ParameterType::WAVEFORM);
    uint8_t wf = std::get<ValueType>(value);

    int idx = waveforms_->findData(wf);

    if ( idx != -1 ){
        QSignalBlocker blocker(waveforms_);
        waveforms_->setCurrentIndex(idx);
    } else {
        qWarning() << "could not set waveform value, enum not found in data" ;
        return ;
    }
}

/*
=========================================
=============== FILTER TYPE =============
=========================================
*/
FilterTypeWidget::FilterTypeWidget(QWidget* parent):
    label_(nullptr),
    type_(nullptr)
{
    QVBoxLayout* layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(Theme::COMPONENT_DETAIL_WIDGET_SPACING);

    // label
    label_ = new QLabel("Filter Type");
    layout->addWidget(label_);

    // populate types
    type_ = new QComboBox(this);
    for ( auto ft : FilterType::getFilterTypes()){
        FilterType f = FilterType(ft);
        type_->addItem(QString::fromStdString(f.toString()),f.to_uint8());
    }

    // set default type
    int idx = type_->findData(GET_PARAMETER_TRAIT_MEMBER(ParameterType::FILTER_TYPE, defaultValue));
    if ( idx != -1 ){
        type_->setCurrentIndex(idx);
    } 
    layout->addWidget(type_);

    // connections
    connect(type_, &QComboBox::currentIndexChanged, this, &ParameterWidget::valueChanged);

    auto p = dynamic_cast<ComponentDetailWidget*>(parent);
    if ( p ){
        connect(this, &ParameterWidget::valueChanged, p, &ComponentDetailWidget::onValueChange);
    }

}

ParameterValue FilterTypeWidget::getValue() const {
    return type_->itemData(type_->currentIndex()).value<uint8_t>();
}

void FilterTypeWidget::setValue(const ParameterValue& value){
    using ValueType = GET_PARAMETER_VALUE_TYPE(ParameterType::FILTER_TYPE);
    uint8_t ft = std::get<ValueType>(value);

    int idx = type_->findData(ft);

    if ( idx != -1 ){
        QSignalBlocker blocker(type_);
        type_->setCurrentIndex(idx);
    } else {
        qWarning() << "could not set filter type value, enum not found in data" ;
        return ;
    }
}

/*
=========================================
================== STATUS ===============
=========================================
*/
StatusWidget::StatusWidget(QWidget* parent):
    label_(nullptr),
    toggle_(nullptr)
{
    QVBoxLayout* layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(Theme::COMPONENT_DETAIL_WIDGET_SPACING);

    // label
    label_ = new QLabel("Status");
    layout->addWidget(label_);

    // create status toggle
    toggle_ = new SwitchWidget(this);
    toggle_->setChecked(true);

    // connections
    connect(toggle_, &QAbstractButton::toggled, this, &ParameterWidget::valueChanged);

    auto p = dynamic_cast<ComponentDetailWidget*>(parent);
    if ( p ){
        connect(this, &ParameterWidget::valueChanged, p, &ComponentDetailWidget::onValueChange);
    }
}

ParameterValue StatusWidget::getValue() const {
    return toggle_->isChecked();
}

void StatusWidget::setValue(const ParameterValue& value){
    using ValueType = GET_PARAMETER_VALUE_TYPE(ParameterType::STATUS);
    auto status = std::get<ValueType>(value);

    toggle_->setChecked(status);
}

/*
=========================================
================= SLIDER =================
=========================================
*/
SliderWidget::SliderWidget(ParameterType p, QWidget* parent): 
    ParameterWidget(parent),
    param_(p),
    label_(nullptr),
    slider_(nullptr),
    valueLabel_(nullptr)
{
    precision_ = GET_PARAMETER_TRAIT_MEMBER(p, uiPrecision);
    setupUI();
    connectSignals();
}

ParameterValue SliderWidget::getValue() const {
    double v = slider_->value() * std::pow(10, -1.0 * precision_);
    switch(param_){
        #define X(name) \
            case ParameterType::name: \
                return ParameterValue{ \
                    static_cast<GET_PARAMETER_VALUE_TYPE(ParameterType::name)>(v) \
                };
            PARAMETER_TYPE_LIST
        #undef X
        default:
            throw std::runtime_error("Parameter of type " +
                GET_PARAMETER_TRAIT_MEMBER(param_, name) +
                " is not in enum. This shouldn't happen.") ;
    }
}

void SliderWidget::setValue(const ParameterValue& value){
    // dispatch for value type, multiplying by precision to properly convert to int slider
    switch(param_){
        #define X(name) \
            case ParameterType::name: \
                slider_->setValue( \
                    scaleByPrecision(std::get<GET_PARAMETER_VALUE_TYPE(ParameterType::name)>(value)) \
                ); \
                break ; 
            PARAMETER_TYPE_LIST
        #undef X
        default:
            qWarning() << "Parameter of type " << 
                GET_PARAMETER_TRAIT_MEMBER(param_, name) 
                << " is not in enum. This shouldn't happen." ;
            break ;
    }
}

void SliderWidget::setupUI(){
    QVBoxLayout* layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(Theme::COMPONENT_DETAIL_WIDGET_SPACING);
    
    // widget label
    label_ = new QLabel(QString::fromStdString(GET_PARAMETER_TRAIT_MEMBER(param_, name)));
    label_->setAlignment(Qt::AlignCenter);
    layout->addWidget(label_);

    // value label
    valueLabel_ = new QLabel();
    valueLabel_->setAlignment(Qt::AlignCenter);
    layout->addWidget(valueLabel_);
    
    // value slider
    slider_ = new QSlider(Qt::Horizontal);
    slider_->setMinimum(scaleByPrecision(GET_PARAMETER_TRAIT_MEMBER(param_, minimum)));
    slider_->setMaximum(scaleByPrecision(GET_PARAMETER_TRAIT_MEMBER(param_,maximum)));  
    slider_->setEnabled(true);
    layout->addWidget(slider_);

    // initialize to default value via dispatch
    switch(param_){
        #define X(name) \
            case ParameterType::name: \
                setValue(static_cast<GET_PARAMETER_VALUE_TYPE(ParameterType::name)>( \
                    GET_PARAMETER_TRAIT_MEMBER(ParameterType::name, defaultValue))); \
                break ;
        PARAMETER_TYPE_LIST
        #undef X
        default:
            qWarning() << "Parameter of type " << 
                GET_PARAMETER_TRAIT_MEMBER(param_, name) 
                << " is not in enum. This shouldn't happen." ;
            slider_->setValue(0);
    }

    updateDisplay();
}

void SliderWidget::connectSignals(){
    connect(slider_, &QSlider::valueChanged, this, [this](){
        updateDisplay();
        emit valueChanged();
    });

    auto p = dynamic_cast<ComponentDetailWidget*>(parent());
    if ( p ){
        connect(this, &ParameterWidget::valueChanged, p, &ComponentDetailWidget::onValueChange);
    }
}

void SliderWidget::updateDisplay(){
    valueLabel_->setText(QString::number(slider_->value() * std::pow(10, -1.0 * precision_)));
}

int SliderWidget::scaleByPrecision(double v) const {
    return v * std::pow(10, precision_);
}