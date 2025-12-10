#pragma once
#include "../packet_header.hpp"
#include <vector>
#include "../../../player_position.hpp"

struct OpponentPositionsPacket {
    UDPPacketHeader header;
    uint8_t positionsCount;
    std::vector<PlayerPosition> positions;
    uint32_t checksum;
};

inline size_t getOpponentPositionsPacketSize(const OpponentPositionsPacket &packet) {
    return sizeof(packet.header)
           + sizeof(packet.positionsCount)
           + sizeof(PlayerPosition) * packet.positionsCount
           + sizeof(packet.checksum);
}

inline PacketBuffer serializeOpponentPositionsPacket(const OpponentPositionsPacket &packet) {
    auto buf = std::make_unique<char[]>(getOpponentPositionsPacketSize(packet));
    size_t currentSize = 0;

    std::memcpy(buf.get(), &packet.header, sizeof(packet.header));
    currentSize += sizeof(packet.header);

    std::memcpy(buf.get() + currentSize, &packet.positionsCount, sizeof(packet.positionsCount));
    currentSize += sizeof(packet.positionsCount);

    for (const auto &position: packet.positions) {
        std::memcpy(buf.get() + currentSize, &position, sizeof(position));
        currentSize += sizeof(position);
    }

    const auto checksum = UDPPacket::calculatePacketChecksum(buf, currentSize + sizeof(packet.checksum));
    std::memcpy(buf.get() + currentSize, &checksum, sizeof(checksum));

    return buf;
}

inline OpponentPositionsPacket deserializeOpponentPositionsPacket(const PacketBuffer &buf, const ssize_t size) {
    OpponentPositionsPacket packet;
    constexpr size_t checksumSize = sizeof(packet.checksum);

    if (size < checksumSize) {
        throw DeserializationError("Received packet too small to contain checksum.");
    }

    const char *checksumAddress = buf.get() + (size - checksumSize);
    std::memcpy(&packet.checksum, checksumAddress, checksumSize);

    const auto checksum = UDPPacket::calculatePacketChecksum(buf, size);
    if (checksum != packet.checksum) {
        throw DeserializationError("Received position packet with invalid checksum.");
    }

    size_t currentOffset = 0;
    std::memcpy(&packet.header, buf.get(), sizeof(packet.header));
    currentOffset += sizeof(packet.header);

    std::memcpy(&packet.positionsCount, buf.get() + currentOffset, sizeof(packet.positionsCount));
    currentOffset += sizeof(packet.positionsCount);

    for (int i = 0; i < packet.positionsCount; ++i) {
        PlayerPosition pos{};
        std::memcpy(&pos, buf.get() + currentOffset, sizeof(PlayerPosition));

        packet.positions.push_back(pos);
        currentOffset += sizeof(PlayerPosition);
    }

    return packet;
}
