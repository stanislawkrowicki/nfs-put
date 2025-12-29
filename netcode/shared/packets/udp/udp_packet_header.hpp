#pragma once

#include <cstdint>
#include "udp_packet_type.hpp"

/* Header MUST ALWAYS be the first field of the packet for deserializing type. */
struct __attribute__((packed)) UDPPacketHeader {
    UDPPacketType type;
    uint8_t payloadSize;
    uint32_t id;
};
