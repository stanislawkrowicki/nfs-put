#pragma once
#include "../tcp_packet_header.hpp"

struct __attribute__((packed)) TimeUntilStartPacket {
    TCPPacketHeader header{
        .type = TCPPacketType::TimeUntilStart,
        .payloadSize = sizeof(uint32_t)
    };
    uint32_t seconds{0};
};

