#pragma once

#include <cstdint>
#include "tcp_packet_type.hpp"

/* Header MUST ALWAYS be the first field of the packet for deserializing type. */
struct __attribute__((packed)) TCPPacketHeader {
    TCPPacketType type;
    uint8_t payloadSize;
};
