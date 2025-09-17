#include "types/SocketType.hpp"
#include <stdexcept>
#include <algorithm>
#include <string>

const std::string socketType2String(SocketType s){
    return std::string(socketStrings[static_cast<int>(s)]);
}

SocketType socketTypeFromString(std::string str) {
    auto it = std::find(socketStrings.begin(), socketStrings.end(), str);
    if (it != socketStrings.end()) {
        return static_cast<SocketType>(std::distance(socketStrings.begin(), it));
    }
    // Handle not found case - throw exception or return sentinel value
    throw std::invalid_argument("Unknown parameter string: " + str);
}