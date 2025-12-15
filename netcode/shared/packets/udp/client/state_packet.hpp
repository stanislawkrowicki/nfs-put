#pragma once
#include <cstdint>
#include <iostream>

#include "../packet_header.hpp"
#include "../udp_packet.hpp"
#include "LinearMath/btTransform.h"

typedef char StateBuffer[80];

constexpr int STATE_PAYLOAD_SIZE = 80;

struct __attribute__((packed)) StatePacket {
    UDPPacketHeader header;
    char payload[STATE_PAYLOAD_SIZE];
    uint32_t checksum;
};

inline btTransform deserializePosition(const PacketBuffer &packet, const size_t size) {
    const auto deserialized = UDPPacket::deserialize<StatePacket>(packet, size);

    btTransform transform;
    btTransformFloatData floatData{};

    std::memcpy(&floatData, deserialized.payload, 64);

    transform.deSerialize(floatData);

    const auto packetType = static_cast<uint32_t>(deserialized.header.type);
    const uint32_t packetId = deserialized.header.id;
    const uint32_t payloadSize = deserialized.header.payloadSize;

    std::cout << std::format("type: {}, id: {}, size: {}", packetType, packetId, payloadSize) << std::endl;;
    return transform;
}
