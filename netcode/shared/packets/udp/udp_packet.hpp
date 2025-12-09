#pragma once
#include <cstdint>
#include <stdexcept>
#include <cstring>
#include <format>
#include <memory>

#include "../../crc32.hpp"

typedef std::unique_ptr<char[]> PacketBuffer;

constexpr int MAX_PAYLOAD_SIZE = 64;

/** TODO: Implement checking checksum */
class UDPPacket {
    template<typename T>
    static uint32_t calculatePacketChecksum(const T &packet) {
        const PacketBuffer buffer = serialize(packet);
        return CRC32::calculate(buffer.get(), sizeof(T) - sizeof(uint32_t));
    }

public:
    template<typename T>
    static PacketBuffer serialize(const T &packet) {
        constexpr auto packetSize = sizeof(T);
        auto buffer = std::make_unique<char[]>(packetSize);
        std::memcpy(buffer.get(), &packet, packetSize);
        return buffer;
    }

    template<typename T>
    static T deserialize(const char *buffer, const size_t size) {
        constexpr auto expectedSize = sizeof(T);

        if (size != expectedSize) {
            throw std::runtime_error(std::format("Deserialization size mismatch. Expected {} bytes, got {}.",
                                                 expectedSize, size));
        }

        T packet{};
        std::memcpy(&packet, buffer, expectedSize);

        return packet;
    }

    template<typename T>
    static T create(const uint8_t type, const uint32_t id, const char *payload, const uint8_t payloadSize) {
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
};
