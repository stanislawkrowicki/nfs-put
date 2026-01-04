#pragma once

#include "../tcp_packet_header.hpp"
#include "../../../../server/tcp_server.hpp"

#include <cstdint>

struct __attribute__((packed)) RaceEndCountdownPacket {
    TCPPacketHeader header{
        .type = TCPPacketType::RaceEndCountDownPacket,
        .payloadSize = sizeof(uint8_t)
    };
    uint8_t lapCount{RACE_END_TIMEOUT};
};
