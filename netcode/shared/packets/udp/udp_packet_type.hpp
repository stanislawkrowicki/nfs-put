#pragma once

#include <cstdint>

enum class UDPPacketType : uint8_t {
    State,
    OpponentStates,
    Ping,
};
