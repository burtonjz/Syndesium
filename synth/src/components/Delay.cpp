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

#include "Delay.hpp"
#include "config/Config.hpp"
#include "params/ParameterMap.hpp"

Delay::Delay(ComponentId id, DelayConfig cfg):
    BaseComponent(id, ComponentType::Delay),
    BaseModule(),
    delay_(cfg.max_delay_sec * Config::get<int>("audio.sample_rate").value())
{
    parameters_->add<ParameterType::DURATION>(cfg.delay_time,true,0,cfg.max_delay_sec);
    parameters_->add<ParameterType::GAIN>(cfg.gain, true);

}

void Delay::calculateSample(){
    double input = 0 ;
    for (auto m : getInputs()){
        input += m->getCurrentSample();
    }
    delay_.write(input);

    double delayDuration = parameters_->getParameter<ParameterType::DURATION>()->getInstantaneousValue() * sampleRate_ ;
    double gain = parameters_->getParameter<ParameterType::GAIN>()->getInstantaneousValue() ;

    buffer_[bufferIndex_] = delay_.read(delayDuration) * gain ;
}