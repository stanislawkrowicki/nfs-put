#pragma once
#include "../tcp_packet_header.hpp"

constexpr int QUEUE_TO_LOBBY= 0;

struct __attribute__((packed)) QueueToLobbyPacket {
    TCPPacketHeader header{
        .type = TCPPacketType::QueueToLobby,
        .payloadSize = QUEUE_TO_LOBBY
    };
};
