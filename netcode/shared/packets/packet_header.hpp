#pragma once

#include <cstdint>

struct __attribute__((packed)) PacketHeader {
    uint8_t type;
    uint8_t payloadSize;
    uint32_t id;
};
