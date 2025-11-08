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

#include "types/ParameterType.hpp"

ParameterType parameterFromString(std::string str) {
    static const std::unordered_map<std::string, ParameterType> str2Type = {
        {ParameterTraits<ParameterType::DEPTH>::name, ParameterType::DEPTH},
        {ParameterTraits<ParameterType::STATUS>::name, ParameterType::STATUS},
        {ParameterTraits<ParameterType::WAVEFORM>::name, ParameterType::WAVEFORM},
        {ParameterTraits<ParameterType::FREQUENCY>::name, ParameterType::FREQUENCY},
        {ParameterTraits<ParameterType::AMPLITUDE>::name, ParameterType::AMPLITUDE},
        {ParameterTraits<ParameterType::GAIN>::name, ParameterType::GAIN},
        {ParameterTraits<ParameterType::DBGAIN>::name, ParameterType::DBGAIN},
        {ParameterTraits<ParameterType::PHASE>::name, ParameterType::PHASE},
        {ParameterTraits<ParameterType::PAN>::name, ParameterType::PAN},
        {ParameterTraits<ParameterType::DETUNE>::name, ParameterType::DETUNE},
        {ParameterTraits<ParameterType::ATTACK>::name, ParameterType::ATTACK},
        {ParameterTraits<ParameterType::DECAY>::name, ParameterType::DECAY},
        {ParameterTraits<ParameterType::SUSTAIN>::name, ParameterType::SUSTAIN},
        {ParameterTraits<ParameterType::RELEASE>::name, ParameterType::RELEASE},
        {ParameterTraits<ParameterType::MIN_VALUE>::name, ParameterType::MIN_VALUE},
        {ParameterTraits<ParameterType::MAX_VALUE>::name, ParameterType::MAX_VALUE},
        {ParameterTraits<ParameterType::FILTER_TYPE>::name, ParameterType::FILTER_TYPE},
        {ParameterTraits<ParameterType::CUTOFF>::name, ParameterType::CUTOFF},
        {ParameterTraits<ParameterType::BANDWIDTH>::name, ParameterType::BANDWIDTH},
        {ParameterTraits<ParameterType::SHELF>::name, ParameterType::SHELF},
        {ParameterTraits<ParameterType::Q_FACTOR>::name, ParameterType::Q_FACTOR},
    };
    
    auto it = str2Type.find(str);
    if (it != str2Type.end()) {
        return it->second;
    }
    throw std::runtime_error("Unknown parameter name: " + str);
}