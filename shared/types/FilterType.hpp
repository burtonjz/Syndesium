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

#ifndef __FILTER_TYPE_HPP_
#define __FILTER_TYPE_HPP_

#include <cstdint>
#include <nlohmann/json.hpp>

using json = nlohmann::json ;

class FilterType {
public:
    enum Filter : uint8_t {
        LowPass,
        HighPass,
        BandPass,
        BandStop,
        PeakingBell,
        LowShelf,
        HighShelf,
        N
    };

    FilterType() = default ;
    // construct from enum
    constexpr FilterType(Filter wf) : filterType_(wf){} 

    // construct from string
    FilterType(std::string_view name){
        if      ( name == "LowPass" )     filterType_ = LowPass  ;
        else if ( name == "HighPass" )    filterType_ = HighPass ;
        else if ( name == "BandPass" )    filterType_ = BandPass ;
        else if ( name == "BandStop" )    filterType_ = BandStop ;
        else if ( name == "PeakingBell" ) filterType_ = PeakingBell ;
        else if ( name == "LowShelf" )    filterType_ = LowShelf ;
        else if ( name == "HighShelf" )   filterType_ =  HighShelf ;
        else throw std::invalid_argument("Unknown filter type: " + std::string(name));
    }

    // allow switch / comparisons
    constexpr operator Filter() const { return filterType_ ; }

    // prevent bool usage e.g., if(FilterType)
    explicit operator bool() const = delete ;

    static std::string toString(FilterType w){
        switch(w){
        case LowPass:     return "LowPass" ;
        case HighPass:    return "HighPass" ;
        case BandPass:    return "BandPass" ;
        case BandStop:    return "BandStop" ;
        case PeakingBell: return "PeakingBell" ;
        case LowShelf:    return "LowShelf" ;
        case HighShelf:   return  "HighShelf" ;
        default:          return "" ;
        };
    }

    std::string toString() const {
        return FilterType::toString(filterType_);
    }

    static std::array<std::string_view, N> getFilterTypes(){
        return {
            "LowPass",
            "HighPass",
            "BandPass",
            "BandStop",
            "PeakingBell",
            "LowShelf",
            "HighShelf"
        } ;
    }

    static Filter from_uint8(uint8_t val){
        return static_cast<Filter>(static_cast<std::underlying_type_t<Filter>>(val));
    }

    uint8_t to_uint8(){
        return static_cast<uint8_t>(filterType_) ;
    }

private:
    Filter filterType_ ;
};

inline void to_json(json& j, const FilterType& w){
        j = w.toString();
    }

inline void from_json(const json& j, FilterType& w){
    w = FilterType(j.get<std::string>());
}

#endif // __FILTER_TYPE_HPP_
