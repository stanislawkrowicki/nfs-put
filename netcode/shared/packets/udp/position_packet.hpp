#pragma once
#include <cstdint>
#include <iostream>

#include "packet_header.hpp"
#include "udp_packet.hpp"
#include "LinearMath/btTransform.h"

constexpr int POSITION_PAYLOAD_SIZE = 64;

struct __attribute__((packed)) PositionPacket {
    UDPPacketHeader header;
    char payload[POSITION_PAYLOAD_SIZE];
    uint32_t checksum;
};

inline btTransform deserializePosition(const PacketBuffer &packet, const size_t size) {
    const auto deserialized = UDPPacket::deserialize<PositionPacket>(packet.get(), size);

    btTransform transform;
    btTransformFloatData floatData{};

    std::memcpy(&floatData, deserialized.payload, 64);

    transform.deSerialize(floatData);

    const uint32_t packetType = deserialized.header.type;
    const uint32_t packetId = deserialized.header.id;
    const uint32_t payloadSize = deserialized.header.payloadSize;

    std::cout << std::format("type: {}, id: {}, size: {}", packetType, packetId, payloadSize) << std::endl;;
    return transform;
}
