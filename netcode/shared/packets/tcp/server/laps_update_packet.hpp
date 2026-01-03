#pragma once

#include "../tcp_packet_header.hpp"

struct __attribute__((packed)) LapsUpdatePacket {
    TCPPacketHeader header{
        .type = TCPPacketType::LapsUpdate,
        .payloadSize = sizeof(uint16_t) + sizeof(uint8_t)
    };

    uint16_t clientId;
    uint8_t laps;
};
