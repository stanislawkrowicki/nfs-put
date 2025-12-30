#pragma once
#include "../udp_packet_header.hpp"
#include <vector>
#include "../../../client_state.hpp"

constexpr ssize_t OPPONENT_STATES_PACKET_SIZE_WITHOUT_DATA = sizeof(UDPPacketHeader)
                                                             + sizeof(uint8_t)
                                                             + sizeof(uint32_t);

struct OpponentStatesPacket {
    UDPPacketHeader header{
        .type = UDPPacketType::PositionResponse,
        .payloadSize = OPPONENT_STATES_PACKET_SIZE_WITHOUT_DATA,
        .id = 0
    };
    uint8_t statesCount{};
    std::vector<ClientState> states{};
    uint32_t checksum{};
};


inline size_t getOpponentStatePacketSize(const OpponentStatesPacket &packet) {
    return sizeof(packet.header)
           + sizeof(packet.statesCount)
           + sizeof(ClientState) * packet.statesCount
           + sizeof(packet.checksum);
}

inline PacketBuffer serializeOpponentState(const OpponentStatesPacket &packet) {
    auto buf = std::make_unique<char[]>(getOpponentStatePacketSize(packet));
    size_t currentSize = 0;

    std::memcpy(buf.get(), &packet.header, sizeof(packet.header));
    currentSize += sizeof(packet.header);

    std::memcpy(buf.get() + currentSize, &packet.statesCount, sizeof(packet.statesCount));
    currentSize += sizeof(packet.statesCount);

    for (const auto &state: packet.states) {
        std::memcpy(buf.get() + currentSize, &state, sizeof(state));
        currentSize += sizeof(state);
    }

    std::memcpy(buf.get() + currentSize, &packet.checksum, sizeof(packet.checksum));

    return buf;
}

inline OpponentStatesPacket deserializeOpponentState(const PacketBuffer &buf, const ssize_t size) {
    OpponentStatesPacket packet;
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

    std::memcpy(&packet.statesCount, buf.get() + currentOffset, sizeof(packet.statesCount));
    currentOffset += sizeof(packet.statesCount);

    for (int i = 0; i < packet.statesCount; ++i) {
        ClientState state{};
        std::memcpy(&state, buf.get() + currentOffset, sizeof(ClientState));

        packet.states.push_back(state);
        currentOffset += sizeof(ClientState);
    }

    return packet;
}
