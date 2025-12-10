#pragma once

#include "../../shared/packets/udp/client/position_packet.hpp"
#include "../../server/udp_server.hpp"

class PositionHandler {
public:
    static void handle(const PositionPacket &packet, const UDPServer *server) {
    }
};
