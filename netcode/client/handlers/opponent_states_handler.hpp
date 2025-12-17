#pragma once

#include "netcode/client/opponent_manager.hpp"
#include "netcode/shared/packets/udp/server/opponent_states_packet.hpp"

class OpponentStatesHandler {
public:
    static void handle(const PacketBuffer &buf, const ssize_t size) {
        auto packet = deserializeOpponentState(buf, size);

        for (const auto [clientId, state]: packet.states) {
            OpponentManager::getInstance().updateOpponentState(clientId, state);
        }
    }
};

