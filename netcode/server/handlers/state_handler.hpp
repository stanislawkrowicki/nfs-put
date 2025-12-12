#pragma once

#include "../../shared/packets/udp/client/position_packet.hpp"
#include "../../server/udp_server.hpp"
#include "../../shared/client_state.hpp"
#include "../../server/loop.hpp"

class StateHandler {
public:
    static void handle(const PositionPacket &packet, const ClientHandle &client) {
        if (packet.header.id < client.lastReceivedPacketId)
            return;

        ClientState state{client.id};
        std::memcpy(state.transform, packet.payload, 64);

        Loop::enqueueStateUpdate(state);
    }
};
