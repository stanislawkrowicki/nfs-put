#pragma once
#include "../tcp_packet_header.hpp"
#include "../../../opponent_info.hpp"

constexpr int START_GAME_PAYLOAD_SIZE = 4;

struct __attribute__((packed)) StartGamePacket {
    TCPPacketHeader header{
        .type = TCPPacketType::StartGame,
        .payloadSize = START_GAME_PAYLOAD_SIZE
    };
    uint8_t gridPosition{};
    PlayerVehicleColor vehicleColor{};
};
