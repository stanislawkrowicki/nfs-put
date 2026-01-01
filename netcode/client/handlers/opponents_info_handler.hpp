#pragma once

#include "netcode/client/opponent_manager.hpp"
#include "netcode/shared/packets/tcp/tcp_packet.hpp"
#include "netcode/shared/packets/tcp/server/opponents_info_packet.hpp"

class OpponentsInfoHandler {
public:
    static void handle(const PacketBuffer &payload, const ssize_t size) {
        const auto opponentInfos = OpponentsInfoPacket::deserialize(payload, size);

        for (const auto &info: opponentInfos) {
            OpponentManager::getInstance().addNewOpponent(info.id, info.gridPosition, info.vehicleColor);
        }
    }
};
