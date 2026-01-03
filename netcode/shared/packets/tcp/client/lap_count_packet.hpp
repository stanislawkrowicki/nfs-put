#pragma once

#include "../tcp_packet_header.hpp"
#include <cstdint>

struct __attribute__((packed)) LapCountPacket {
    TCPPacketHeader header{
        .type = TCPPacketType::LapCount,
        .payloadSize = sizeof(uint8_t)
    };
    uint8_t lapCount{};
};
