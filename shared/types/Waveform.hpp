#ifndef __WAVEFORM_HPP_
#define __WAVEFORM_HPP_

#include <nlohmann/json.hpp>
#include <stdexcept>
#include <string>
#include <string_view>
#include <array>
#include <cstdint> 
#include <type_traits>
#include <optional>

using json = nlohmann::json ;

class Waveform {
public:
    enum Wave : uint8_t {
        SINE = 0, 
        SQUARE,
        TRIANGLE,
        SAW,
        NOISE,
        N
    };

    Waveform() = default ;
    // construct from enum
    constexpr Waveform(Wave wf) : waveform_(wf){} 

    // construct from string
    Waveform(std::string_view name){
        if      (name == "SINE")     waveform_ = SINE ;
        else if (name == "SQUARE")   waveform_ = SQUARE ;
        else if (name == "TRIANGLE") waveform_ = TRIANGLE ;
        else if (name == "SAW")      waveform_ = SAW ;
        else if (name == "NOISE")    waveform_ = NOISE ;
        else throw std::invalid_argument("Unknown waveform: " + std::string(name));
    }

    // allow switch / comparisons
    constexpr operator Wave() const { return waveform_ ; }

    // prevent bool usage e.g., if(Waveform)
    explicit operator bool() const = delete ;

    static std::string toString(Waveform w){
        switch(w){
        case SINE:     return "SINE" ;
        case SQUARE:   return "SQUARE" ;
        case TRIANGLE: return "TRIANGLE" ;
        case SAW:      return "SAW" ;
        case NOISE:    return "NOISE" ;
        default:       return "" ;
        };
    }

    std::string toString() const {
        return Waveform::toString(waveform_);
    }

    static std::array<std::string_view, N> getWaveforms(){
        return { "SINE", "SQUARE", "TRIANGLE", "SAW", "NOISE"} ;
    }

    static Wave from_uint8(uint8_t val){
        return static_cast<Wave>(static_cast<std::underlying_type_t<Wave>>(val));
    }

private:
    Wave waveform_ ;
};

inline void to_json(json& j, const Waveform& w){
        j = w.toString();
    }

inline void from_json(const json& j, Waveform& w){
    w = Waveform(j.get<std::string>());
}


// support json string serialization for Waveform class
// NLOHMANN_JSON_NAMESPACE_BEGIN
//     template <>
//     struct adl_serializer<std::optional<Waveform>> {
//         static inline void to_json(json& j, const std::optional<Waveform>& opt) {
//             j = opt.has_value() ? opt->toString() : nullptr ;
//         }

//         static inline void from_json(const json& j, std::optional<Waveform>& opt) {
//             if (j.is_null()) {
//                 opt = std::nullopt;
//             } else if (j.is_string()) {
//                 opt = Waveform(j.get<std::string>());
//             } else {
//                 throw std::invalid_argument("Waveform must be string or null");
//             }
//         }
//     };
// NLOHMANN_JSON_NAMESPACE_END


#endif // __WAVEFORM_HPP_