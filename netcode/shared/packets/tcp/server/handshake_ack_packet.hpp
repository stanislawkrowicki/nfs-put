#pragma once

#include "../tcp_packet_header.hpp"

constexpr int HANDSHAKE_ACK_PAYLOAD_SIZE = 0;

struct __attribute__((packed)) HandshakeAckPacket {
    TCPPacketHeader header{
        .type = TCPPacketType::HandshakeAck,
        .payloadSize = HANDSHAKE_ACK_PAYLOAD_SIZE,
    };
    char payload[HANDSHAKE_ACK_PAYLOAD_SIZE];
};
