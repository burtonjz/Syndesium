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

#ifndef __MODULATOR_HPP_
#define __MODULATOR_HPP_

#include "midi/MidiNote.hpp"
#include "types/ModulatorType.hpp"
#include "types/ParameterType.hpp"
#include "params/ModulationParameter.hpp"
#include "containers/RTMap.hpp"
#include "containers/AtomicFloat.hpp"

#include <memory>
#include <set>
#include <variant>

// forward declaration
class ParameterMap ; 

using  ModulationData = RTMap<ModulationParameter, AtomicFloat, N_MODULATION_PARAMETERS> ;

struct ModulationContext {
    const MidiNote* note = nullptr ;
};

/**
 * @brief base class for all Modulators.
*/
class BaseModulator {
protected:
    ModulatorType type_ ;
    std::unique_ptr<ParameterMap> parameters_ ; 
    std::set<ModulationParameter> requiredParams_ ; 

public:
    BaseModulator(ModulatorType typ);

    virtual ~BaseModulator() = default ;

    virtual double modulate(double value, ModulationData* mdat ) const = 0 ;

    virtual ModulatorType getType() const ;

    ParameterMap* getParameters();

    virtual void setParameterModulation(ParameterType p, BaseModulator* m, ModulationData d = {} );

    virtual const std::set<ModulationParameter>& getRequiredModulationParameters() const {
        return requiredParams_ ;
    }

    void tick();

    bool setParameterValue(ParameterType t, const json& value);

};

#endif // __MODULATOR_HPP_


#include "containers/RTMap.hpp"

