#pragma once
#include <cstdint>
#include <iostream>

#include "../udp_packet_header.hpp"
#include "../udp_packet.hpp"
#include "LinearMath/btTransform.h"

constexpr int RACE_START_PAYLOAD_SIZE = 81;

typedef char StateBuffer[RACE_START_PAYLOAD_SIZE];

struct __attribute__((packed)) StatePacket {
    UDPPacketHeader header;
    char payload[RACE_START_PAYLOAD_SIZE];
    uint32_t checksum;
};
