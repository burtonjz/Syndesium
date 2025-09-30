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

#ifndef __MODULATION_PARAMETER_HPP_
#define __MODULATION_PARAMETER_HPP_

/**
 * @brief valid key values for handling additional modulation variables
 * 
 * The parameter class stores a modulation function in it that can receive a map as a parameter so that additional context can be given to modulators
 * while still allowing the modulation function definition to be standardized. These keys define various modulation function variables.
*/
enum class ModulationParameter {
    MIDI_NOTE,
    INITIAL_VALUE, 
    LAST_VALUE,
    N_PARAMETERS
};

constexpr int N_MODULATION_PARAMETERS = static_cast<int>(ModulationParameter::N_PARAMETERS);

#endif // __MODULATION_PARAMETER_HPP_