#pragma once

#pragma once
#include <cstdint>
#include <stdexcept>
#include <cstring>
#include <format>
#include <memory>

#include "tcp_packet_header.hpp"
#include "tcp_packet_type.hpp"
#include "../../crc32.hpp"
#include "../../deserialization_error.hpp"

typedef std::unique_ptr<char[]> PacketBuffer;

constexpr int MAX_TCP_PAYLOAD_SIZE = 255;

class TCPPacket {
public:
    template<typename T>
    static PacketBuffer serialize(const T &packet) {
        constexpr auto packetSize = sizeof(T);
        return serialize(packet, packetSize);
    }

    template<typename T>
    static PacketBuffer serialize(const T &packet, const uint16_t packetSize) {
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
    static std::pair<T, uint16_t> create(const TCPPacketType type, const char *payload, const uint16_t payloadSize) {
        if (payloadSize > MAX_TCP_PAYLOAD_SIZE) {
            throw std::length_error("Payload size exceeds MAX_TCP_PAYLOAD_SIZE");
        }

        T packet{};

        packet.header.type = type;
        packet.header.payloadSize = payloadSize;

        std::memcpy(packet.payload, payload, payloadSize);

        return {packet, sizeof(TCPPacketHeader) + payloadSize};
    }
};
