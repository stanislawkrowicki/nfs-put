#pragma once

#include <cstdint>

#include "../udp_packet_header.hpp"

constexpr int HANDSHAKE_PAYLOAD_SIZE = sizeof(uint16_t);

struct __attribute__((packed)) HandshakePacket {
    UDPPacketHeader header{
        .type = UDPPacketType::Handshake,
        .payloadSize = HANDSHAKE_PAYLOAD_SIZE,
        .id = 0,
    };

    char payload[HANDSHAKE_PAYLOAD_SIZE];
    uint32_t checksum{};
};
