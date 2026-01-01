#pragma once

constexpr int RACE_START_PAYLOAD_SIZE = 0;

struct __attribute__((packed)) RaceStartPacket {
    TCPPacketHeader header{
        .type = TCPPacketType::StartGame,
        .payloadSize = RACE_START_PAYLOAD_SIZE
    };
};
