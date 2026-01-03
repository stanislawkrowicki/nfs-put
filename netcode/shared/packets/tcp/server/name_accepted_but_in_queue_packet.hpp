#pragma once
#include "../tcp_packet_header.hpp"

constexpr int NAME_ACCEPTED_BUT_IN_QUEUE_PAYLOAD_SIZE = 0;

struct __attribute__((packed)) NameAcceptedButInQueuePacket {
    TCPPacketHeader header{
        .type = TCPPacketType::NameAcceptedButInQueue,
        .payloadSize = NAME_ACCEPTED_BUT_IN_QUEUE_PAYLOAD_SIZE
    };
};
