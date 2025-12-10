#pragma once
#include "udp_server.hpp"
#include "../shared/packets/udp/udp_packet.hpp"
#include <vector>

#include "../shared/packets/udp/server/opponent_positions_packet.hpp"

class Loop {
    static constexpr int TICK_RATE = 32;
    static bool exit;

    static std::shared_ptr<UDPServer> server;

    static std::vector<PlayerPosition> positionsToUpdate;

    static void sendMessageToAll();

    static void sendPositions();

    static OpponentPositionsPacket packPositionsBatch(const std::vector<PlayerPosition> &batch);

public:
    static void run(const std::shared_ptr<UDPServer> &udpServer);

    static void enqueuePositionUpdate(const PlayerPosition &position);
};
