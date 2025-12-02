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

#ifndef __MODULE_POLYOSCILLATOR_HPP_
#define __MODULE_POLYOSCILLATOR_HPP_

#include "midi/MidiNote.hpp"
#include "core/BaseModule.hpp"
#include "midi/MidiEventListener.hpp"
#include "containers/RTMap.hpp"
#include "containers/FixedPool.hpp"
#include "components/Oscillator.hpp"
#include "types/ParameterType.hpp"
#include "configs/PolyOscillatorConfig.hpp"
#include <cstdint>


class PolyOscillator : public BaseModule, public MidiEventListener {
private:
    RTMap<uint8_t, Oscillator*, 128> children_ ;
    FixedPool<Oscillator, 128> childPool_ ;

    // modulation
    std::array<BaseModulator*, N_PARAMETER_TYPES> modulators_ ;
    std::array<ModulationData, N_PARAMETER_TYPES> modulationData_ ;
    
public:
    // Constructors
    PolyOscillator(ComponentId id, PolyOscillatorConfig cfg);
    
    // Module Overrides
    bool isGenerative() const override ;
    void calculateSample() override ;
    void clearBuffer() override ;
    void tick() override ;

    // MidiEventListener Overrides
    void onKeyPressed(const ActiveNote* note, bool rePress = false) override ;
    void onKeyReleased(ActiveNote anote) override ;
    void onKeyOff(ActiveNote anote) override ;

    // BaseComponent Overrides
    void updateParameters() override ;
    BaseModulator* getParameterModulator(ParameterType p) const  override ;
    void onSetParameterModulation(ParameterType p, BaseModulator* m, ModulationData d = {} ) override ;
    void onRemoveParameterModulation(ParameterType p) override ;

    void updateGain();
private:
    void updateModulationInitialValue(Oscillator* osc);

};  

#endif // __MODULE_POLYOSCILLATOR_HPP_