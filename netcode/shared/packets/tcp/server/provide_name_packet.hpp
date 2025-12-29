#pragma once
#include "../tcp_packet_header.hpp"

constexpr int PROVIDE_NAME_PAYLOAD_SIZE = 0;

struct __attribute__((packed)) ProvideNamePacket {
    TCPPacketHeader header{
        .type = TCPPacketType::ProvideName,
        .payloadSize = PROVIDE_NAME_PAYLOAD_SIZE
    };
};
