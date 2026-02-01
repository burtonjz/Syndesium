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

