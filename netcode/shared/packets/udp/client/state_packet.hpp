#pragma once
#include <cstdint>
#include <iostream>

#include "../packet_header.hpp"
#include "../udp_packet.hpp"
#include "LinearMath/btTransform.h"

constexpr int STATE_PAYLOAD_SIZE = 81;

typedef char StateBuffer[STATE_PAYLOAD_SIZE];

struct __attribute__((packed)) StatePacket {
    UDPPacketHeader header;
    char payload[STATE_PAYLOAD_SIZE];
    uint32_t checksum;
};
