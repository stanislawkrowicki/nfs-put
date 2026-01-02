#pragma once
#include "../tcp_packet_header.hpp"

constexpr int RACE_START_COUNTDOWN_PAYLOAD_SIZE = sizeof(uint8_t);

struct __attribute__((packed)) RaceStartCountdownPacket {
    TCPPacketHeader header{
        .type = TCPPacketType::RaceStartCountdown,
        .payloadSize = RACE_START_COUNTDOWN_PAYLOAD_SIZE
    };
    uint8_t secondsUntilStart{};
};
