#pragma once
#include "netcode/shared/packets/tcp/tcp_packet_header.hpp"

constexpr int UDP_INFO_PAYLOAD_SIZE = 32;

struct __attribute__((packed)) UdpInfoPacket {
    TCPPacketHeader header{
        .type = TCPPacketType::UdpInfo,
        .payloadSize = UDP_INFO_PAYLOAD_SIZE
    };
    uint16_t port{};
};
