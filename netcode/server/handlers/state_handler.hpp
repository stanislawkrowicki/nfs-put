#pragma once

#include "../../shared/packets/udp/client/state_packet.hpp"
#include "../../server/udp_server.hpp"
#include "../../shared/client_state.hpp"
#include "../../server/loop.hpp"

class StateHandler {
public:
    static void handle(const StatePacket &packet, ClientHandle &client) {
        if (packet.header.id < client.lastReceivedPacketId)
            return;

        ClientState state{client.id};
        std::memcpy(state.state, packet.payload, RACE_START_PAYLOAD_SIZE);

        Loop::enqueueStateUpdate(state);

        client.lastReceivedPacketId++;
    }
};
