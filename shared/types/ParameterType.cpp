#include "types/ParameterType.hpp"
#include <algorithm>

const std::string parameter2String(ParameterType p){
    return std::string(parameterStrings[static_cast<int>(p)]);
}

ParameterType parameterFromString(std::string str) {
    auto it = std::find(parameterStrings.begin(), parameterStrings.end(), str);
    if (it != parameterStrings.end()) {
        return static_cast<ParameterType>(std::distance(parameterStrings.begin(), it));
    }
    // Handle not found case - throw exception or return sentinel value
    throw std::invalid_argument("Unknown parameter string: " + str);
}