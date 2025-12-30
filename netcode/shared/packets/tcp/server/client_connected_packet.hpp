#pragma once
#include "../tcp_packet_header.hpp"

constexpr int CONNNAME_PAYLOAD_SIZE = 32;

struct __attribute__((packed)) ClientConnectedPacket {
    TCPPacketHeader header{
        .type = TCPPacketType::ClientConnected,
        .payloadSize = CONNNAME_PAYLOAD_SIZE
    };
    char payload[CONNNAME_PAYLOAD_SIZE]{};
};
