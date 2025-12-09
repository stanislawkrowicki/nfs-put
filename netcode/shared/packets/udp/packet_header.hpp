#pragma once

#include <cstdint>

struct __attribute__((packed)) UDPPacketHeader {
    UDPPacketType type;
    uint8_t payloadSize;
    uint32_t id;
};
