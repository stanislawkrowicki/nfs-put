#pragma once

#include "../udp_packet_header.hpp"

constexpr int PING_PAYLOAD_SIZE = 0;

/* This packet is used as an empty packet to open the firewall */
struct __attribute__((packed)) PingPacket {
    UDPPacketHeader header{
        .type = UDPPacketType::Ping,
        .payloadSize = PING_PAYLOAD_SIZE,
        .id = 0
    };
    char payload[PING_PAYLOAD_SIZE];
    uint32_t checksum{};
};
