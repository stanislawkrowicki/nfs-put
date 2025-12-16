#pragma once
#include <cstdint>
#include <stdexcept>
#include <cstring>
#include <format>
#include <memory>

#include "udp_packet_type.hpp"
#include "../../crc32.hpp"
#include "../../deserialization_error.hpp"

typedef std::unique_ptr<char[]> PacketBuffer;

constexpr int MAX_PAYLOAD_SIZE = 81;

class UDPPacket {
public:
    template<typename T>
    static PacketBuffer serialize(const T &packet) {
        constexpr auto packetSize = sizeof(T);
        auto buffer = std::make_unique<char[]>(packetSize);
        std::memcpy(buffer.get(), &packet, packetSize);
        return buffer;
    }

    template<typename T>
    static T deserialize(const PacketBuffer &buffer, const size_t size) {
        constexpr auto expectedSize = sizeof(T);

        if (size != expectedSize) {
            throw DeserializationError(std::format("Deserialization size mismatch. Expected {} bytes, got {}.",
                                                   expectedSize, size));
        }

        T packet{};
        std::memcpy(&packet, buffer.get(), expectedSize);

        return packet;
    }

    template<typename T>
    static T create(const UDPPacketType type, const uint32_t id, const char *payload, const uint8_t payloadSize) {
        if (payloadSize > MAX_PAYLOAD_SIZE) {
            throw std::length_error("Payload size exceeds MAX_PAYLOAD_SIZE");
        }

        T packet{};

        packet.header.type = type;
        packet.header.payloadSize = payloadSize;
        packet.header.id = id;

        std::memcpy(packet.payload, payload, payloadSize);

        packet.checksum = calculatePacketChecksum(packet);

        return packet;
    }

    static bool validate(const PacketBuffer &packet, const size_t size) {
        constexpr size_t checksumSize = sizeof(uint32_t);

        if (size < checksumSize)
            return false;

        uint32_t expectedChecksum;
        const char *checksumAddress = packet.get() + (size - checksumSize);

        std::memcpy(&expectedChecksum, checksumAddress, checksumSize);
        const auto checksum = calculatePacketChecksum(packet, size);

        return checksum == expectedChecksum;
    }

    template<typename T>
    static uint32_t calculatePacketChecksum(const T &packet) {
        const PacketBuffer buffer = serialize(packet);
        return calculatePacketChecksum(buffer, sizeof(T));
    }

    /* The size of the buffer you provide must include the checksum field (4 bytes) */
    static uint32_t calculatePacketChecksum(const PacketBuffer &buffer, const size_t size) {
        return CRC32::calculate(buffer.get(), size - sizeof(uint32_t));
    }
};
