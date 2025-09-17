#ifndef __SHARED_SOCKET_TYPE_HPP_
#define __SHARED_SOCKET_TYPE_HPP_

#include <array>
#include <string_view>

enum class SocketType {
    ModulationInput,
    ModulationOutput,
    SignalInput,
    SignalOutput,
    MidiInput,
    MidiOutput,
    N_SOCKET_TYPES
};

constexpr int N_SOCKET_TYPES = static_cast<int>(SocketType::N_SOCKET_TYPES) ;

constexpr std::array<std::string_view, N_SOCKET_TYPES> socketStrings({
    "Modulation Input",
    "Modulation Output",
    "Signal Input",
    "Signal Output",
    "MIDI Input",
    "MIDI Output"
});

const std::string socketType2String(SocketType s);
SocketType socketTypeFromString(std::string str);
#endif // __SHARED_SOCKET_TYPE_HPP_