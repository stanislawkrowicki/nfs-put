#pragma once

#include "../../shared/packets/udp/client/position_packet.hpp"
#include "../../server/udp_server.hpp"
#include "../../shared/player_position.hpp"
#include "../../server/loop.hpp"

class PositionHandler {
public:
    static void handle(const PositionPacket &packet, const ClientHandle &client) {
        if (packet.header.id < client.lastReceivedPacketId)
            return;

        const PlayerPosition position{client.id, *packet.payload};
        Loop::enqueuePositionUpdate(position);
    }
};
