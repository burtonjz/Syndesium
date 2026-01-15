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

#ifndef __COLLECTION_TYPE_HPP_
#define __COLLECTION_TYPE_HPP_

#include <cstdint>
#include <stdexcept>
#include <string_view>
#include <array>
#include <nlohmann/json.hpp>

using json = nlohmann::json ;

class CollectionType {
public:
    enum Type : uint8_t {
        SEQUENCER,
        GENERIC,
        N
    };

    CollectionType() = default ;
    // construct from enum
    constexpr CollectionType(Type c) : type_(c){} 

    // construct from string
    CollectionType(std::string_view name){
        if      (name == "sequencer") type_ = SEQUENCER ;
        else if (name == "generic")   type_ = GENERIC ;
        else throw std::invalid_argument("Unknown collection type: " + std::string(name));
    }

    // allow switch / comparisons
    constexpr operator Type() const { return type_ ; }

    // prevent bool usage e.g., if(CollectionType)
    explicit operator bool() const = delete ;

    static std::string toString(CollectionType c){
        switch(c){
        case SEQUENCER: return "sequencer" ;
        case GENERIC:   return "generic" ;
        default:       return "" ;
        };
    }

    std::string toString() const {
        return CollectionType::toString(type_);
    }

    static std::array<std::string_view, N> getCollectionTypes(){
        return { "sequencer", "generic"} ;
    }

    static Type from_uint8(uint8_t val){
        return static_cast<Type>(static_cast<std::underlying_type_t<Type>>(val));
    }

    uint8_t to_uint8(){
        return static_cast<uint8_t>(type_) ;
    }

private:
    Type type_ ;
};

inline void to_json(json& j, const CollectionType& c){
        j = c.toString();
    }

inline void from_json(const json& j, CollectionType& c){
    c = CollectionType(j.get<std::string>());
}


#endif // __COLLECTION_TYPE_HPP_