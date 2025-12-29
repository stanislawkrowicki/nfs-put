#pragma once
#include "../tcp_packet_header.hpp"

constexpr int NAME_ACCEPTED_PAYLOAD_SIZE = 0;

struct __attribute__((packed)) NameAcceptedPacket {
    TCPPacketHeader header{
        .type = TCPPacketType::NameAccepted,
        .payloadSize = NAME_ACCEPTED_PAYLOAD_SIZE
    };
};
