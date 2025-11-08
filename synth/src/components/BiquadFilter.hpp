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
#ifndef __BIQUAD_FILTER_HPP_
#define __BIQUAD_FILTER_HPP_

#include "core/BaseModule.hpp"
#include "core/BaseModulator.hpp"

#include "configs/BiquadFilterConfig.hpp"

class BiquadFilter : public BaseModule, public BaseModulator {
private:
    double sampleRate_ ;
    std::array<double,2> lastInputs_ ;
    std::array<double,2> lastOutputs_ ;
    std::array<double,5> coefficients_ ;

    void calculateCoefficients();
    inline double getCurrentOutput(double x0, double x1, double x2, double y1, double y2) const ;

public:
    BiquadFilter(ComponentId id, BiquadFilterConfig cfg);

    double modulate(double value, ModulationData* mData ) const override ;

    void calculateSample() override ;
    void tick() override ;
};

#endif // __BIQUAD_FILTER_HPP_
