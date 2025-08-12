#include "types/ParameterType.hpp"

const std::string& parameter2String(ParameterType p){
    return parameterStrings[static_cast<int>(p)];
}