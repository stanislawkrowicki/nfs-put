#pragma once

constexpr int START_GAME_PAYLOAD_SIZE = 1;

struct __attribute__((packed)) StartGamePacket {
    TCPPacketHeader header{
        .type = TCPPacketType::StartGame,
        .payloadSize = START_GAME_PAYLOAD_SIZE
    };
    uint8_t gridPosition{};
};
