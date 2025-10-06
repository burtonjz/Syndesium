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

#include "core/BaseComponent.hpp"
#include "midi/MidiNote.hpp"
#include "types/ParameterType.hpp"
#include "params/ModulationParameter.hpp"
#include "containers/RTMap.hpp"
#include "containers/AtomicFloat.hpp"

#include <memory>
#include <set>
#include <variant>

struct ModulationContext {
    const MidiNote* note = nullptr ;
};

/**
 * @brief base class for all Modulators.
*/
class BaseModulator : public virtual BaseComponent {
protected:
    std::set<ModulationParameter> requiredParams_ ; 

public:
    BaseModulator(){};

    virtual ~BaseModulator() = default ;

    virtual double modulate(double value, ModulationData* mdat ) const = 0 ;

    virtual const std::set<ModulationParameter>& getRequiredModulationParameters() const {
        return requiredParams_ ;
    }
};

#endif // __MODULATOR_HPP_
