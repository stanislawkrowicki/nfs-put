#pragma once
#include "../tcp_packet_header.hpp"

constexpr int MAX_NAME_PAYLOAD_SIZE = 32;

struct __attribute__((packed)) NamePacket {
    TCPPacketHeader header{
        .type = TCPPacketType::Name,
        .payloadSize = MAX_NAME_PAYLOAD_SIZE
    };
    char payload[MAX_NAME_PAYLOAD_SIZE]{};
};
