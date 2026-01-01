#pragma once
#include "../../../opponent_info.hpp"
#include "../tcp_packet.hpp"
#include "../tcp_packet_header.hpp"
#include <vector>

constexpr int MAX_OPPONENTS_INFO_PAYLOAD_SIZE = MAX_TCP_PAYLOAD_SIZE;

inline void throwOpponentInfoSizeMismatch(const size_t &offset, const size_t &size) {
    throw DeserializationError(std::format("Tried to read OpponentInfo data at offset {}, but the size is {}", offset,
                                           size));
}

struct __attribute__((packed)) OpponentsInfoPacket {
    TCPPacketHeader header{
        .type = TCPPacketType::OpponentsInfo,
        .payloadSize = 0
    };
    /* Structure: id(2), vehicleColor(3), nicknameLength(1), nickname(nicknameLength) */
    char payload[MAX_OPPONENTS_INFO_PAYLOAD_SIZE]{};

    explicit OpponentsInfoPacket(const std::vector<OpponentInfo> &opponentsInfo) {
        size_t offset = 0;

        for (const auto &[id, vehicleColor, nickname]: opponentsInfo) {
            std::memcpy(payload + offset, &id, sizeof(id));
            offset += sizeof(id);

            std::memcpy(payload + offset, &vehicleColor, sizeof(vehicleColor));
            offset += sizeof(vehicleColor);

            const auto nicknameLength = static_cast<uint8_t>(nickname.length());
            std::memcpy(payload + offset, &nicknameLength, 1);
            offset += 1;

            std::memcpy(payload + offset, nickname.c_str(), nicknameLength);
            offset += nicknameLength;
        }

        if (offset > MAX_OPPONENTS_INFO_PAYLOAD_SIZE)
            throw std::runtime_error("Tried to serialize OpponentsInfoPacket that's too big");

        header.payloadSize = offset;
    }

    static std::vector<OpponentInfo> deserialize(const PacketBuffer &data, const uint16_t size) {
        std::vector<OpponentInfo> result;
        size_t offset = 0;

        const auto dataPtr = data.get();

        while (offset < size) {
            OpponentInfo info;

            if (offset + sizeof(info.id) > size) throwOpponentInfoSizeMismatch(offset, size);
            std::memcpy(&info.id, dataPtr + offset, sizeof(info.id));
            offset += sizeof(info.id);

            if (offset + sizeof(info.vehicleColor) > size) throwOpponentInfoSizeMismatch(offset, size);
            std::memcpy(&info.vehicleColor, dataPtr + offset, sizeof(info.vehicleColor));
            offset += sizeof(info.vehicleColor);

            if (offset + 1 > size) throwOpponentInfoSizeMismatch(offset, size);
            const auto nameLen = static_cast<uint8_t>(dataPtr[offset]);
            offset += 1;

            if (offset + nameLen > size) throwOpponentInfoSizeMismatch(offset, size);
            info.nickname = std::string(dataPtr + offset, nameLen);
            offset += nameLen;

            result.push_back(info);
        }

        return result;
    }
};

