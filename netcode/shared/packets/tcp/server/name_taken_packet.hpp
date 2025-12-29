#pragma once
#include "../tcp_packet_header.hpp"

constexpr int NAME_TAKEN_PAYLOAD_SIZE = 0;

struct __attribute__((packed)) NameTakenPacket {
    TCPPacketHeader header{
        .type = TCPPacketType::NameTaken,
        .payloadSize = NAME_TAKEN_PAYLOAD_SIZE
    };
};
