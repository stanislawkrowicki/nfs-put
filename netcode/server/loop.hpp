#pragma once
#include "udp_server.hpp"
#include "../shared/packets/udp/udp_packet.hpp"
#include <vector>

class Loop {
    static constexpr int TICK_RATE = 32;
    static bool exit;

    static std::shared_ptr<UDPServer> server;

    static std::vector<PacketBuffer> packetsToSend;

    static void sendMessageToAll();

public:
    static void run(const std::shared_ptr<UDPServer> &udpServer);

    static void enqueuePacket();
};
