#pragma once
#include "netcode/shared/packets/tcp/tcp_packet_header.hpp"

constexpr int NAME_PAYLOAD_SIZE = 32;

struct __attribute__((packed)) NamePacket {
    TCPPacketHeader header{
        .type = TCPPacketType::Name,
        .payloadSize = NAME_PAYLOAD_SIZE
    };
    char payload[NAME_PAYLOAD_SIZE]{};
};
