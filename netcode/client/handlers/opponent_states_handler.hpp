#pragma once

#include "netcode/client/opponent_manager.hpp"
#include "netcode/shared/packets/udp/server/opponent_positions_packet.hpp"

class OpponentStatesHandler {
public:
    static void handle(const PacketBuffer &buf, const ssize_t size, const uint16_t localClientId) {
        auto packet = deserializeOpponentPositions(buf, size);

        for (const auto [clientId, state]: packet.states) {
            /* TODO: See Loop::sendPositions() todo */
            if (clientId == localClientId) continue;

            OpponentManager::getInstance().updateOpponent(clientId, state);
        }
    }
};

