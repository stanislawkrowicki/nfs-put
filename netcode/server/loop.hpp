#pragma once
#include "udp_server.hpp"
#include "../shared/packets/udp/udp_packet.hpp"
#include <vector>

#include "../shared/packets/udp/server/opponent_states_packet.hpp"

class Loop {
    static constexpr int TICK_RATE = 32;
    static bool exit;

    static std::shared_ptr<UDPServer> server;

    static std::unordered_map<uint16_t, ClientState> latestClientStates;

    static void sendMessageToAll();

    static void sendLatestStates();

    static OpponentStatesPacket packStatesBatch(const std::vector<ClientState> &batch);

public:
    static void run(const std::shared_ptr<UDPServer> &udpServer);

    static void enqueueStateUpdate(const ClientState &state);
};
