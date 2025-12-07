#pragma once
#include "udp_server.hpp"

class Loop {
private:
    UDPServer server;
    const int TICK_RATE = 32;
    bool exit;

    void sendMessageToAll() const;

public:
    explicit Loop(const UDPServer &server);
};
