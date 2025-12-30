#pragma once
#include "../tcp_packet_header.hpp"
#include <vector>
#include <string>
#include <cstring>
#include <memory>

struct LobbyClientListPacket {
    TCPPacketHeader header{
        .type = TCPPacketType::LobbyClientList,
        .payloadSize = 0
    };
    std::unique_ptr<char[]> payload;

    LobbyClientListPacket(const std::vector<std::string>& nicks, const std::string& excludeNick) {
        size_t totalSize = 0;
        for (const auto& nick : nicks) {
            if (nick != excludeNick)
                totalSize += nick.size() + 1;
        }

        header.payloadSize = static_cast<uint16_t>(totalSize);
        payload = std::make_unique<char[]>(totalSize);

        size_t offset = 0;
        for (const auto& nick : nicks) {
            if (nick == excludeNick) continue;
            std::memcpy(payload.get() + offset, nick.c_str(), nick.size());
            offset += nick.size();
            payload[offset++] = '\0';
        }
    }
};

