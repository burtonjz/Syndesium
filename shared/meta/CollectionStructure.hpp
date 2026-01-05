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

#ifndef __COLLECTION_STRUCTURE_HPP_
#define __COLLECTION_STRUCTURE_HPP_

#include "types/ParameterType.hpp"

#include <stdexcept>
#include <vector>

enum class CollectionStructure {
    INDEPENDENT, // each element may be updated independently
    GROUPED, // elements within collection are treated as groups of N
    SYNCHRONIZED // multiple collections must have same length and parallel indices
};

struct CollectionDescriptor {
    std::vector<ParameterType> types ;
    CollectionStructure structure ;
    std::string name ;
    size_t groupSize = 0 ; // for GROUPED only

    static CollectionDescriptor Independent(
        ParameterType type,
        const std::string& name
    ){
        return {
            {type},
            CollectionStructure::INDEPENDENT,
            name,
            0
        };
    }

    static CollectionDescriptor Grouped(
        ParameterType type,
        const std::string& name,
        size_t groupSize
    ){
        if (groupSize < 2){
            throw std::invalid_argument("Grouped collections must have size > 1");
        }
        return {
            {type},
            CollectionStructure::GROUPED,
            name,
            groupSize
        };
    }

    static CollectionDescriptor Synchronized(
        const std::vector<ParameterType>& types,
        const std::string& name
    ){
        if (types.size() < 2){
            throw std::invalid_argument("Synchronized collections require at least 2 parameter types");
        }
        return {
            types,
            CollectionStructure::SYNCHRONIZED,
            name
        };
    }

};

#endif // __COLLECTION_STRUCTURE_HPP_