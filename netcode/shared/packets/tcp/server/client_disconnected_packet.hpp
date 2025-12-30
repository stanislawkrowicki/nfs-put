#pragma once
#include "../tcp_packet_header.hpp"

constexpr int DISCONNNAME_PAYLOAD_SIZE = 32;

struct __attribute__((packed)) ClientDisconnectedPacket {
    TCPPacketHeader header{
        .type = TCPPacketType::ClientDisconnected,
        .payloadSize = DISCONNNAME_PAYLOAD_SIZE
    };
    char payload[DISCONNNAME_PAYLOAD_SIZE]{};
};
