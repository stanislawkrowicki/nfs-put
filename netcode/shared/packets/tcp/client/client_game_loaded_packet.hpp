#pragma once
#include "netcode/shared/packets/tcp/tcp_packet_header.hpp"

constexpr int CLIENT_GAME_LOADED_PAYLOAD_SIZE = 0;

struct __attribute__((packed)) ClientGameLoadedPacket {
    TCPPacketHeader header{
        .type = TCPPacketType::ClientGameLoaded,
        .payloadSize = CLIENT_GAME_LOADED_PAYLOAD_SIZE
    };
};
