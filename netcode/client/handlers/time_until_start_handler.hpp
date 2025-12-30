#pragma once
#include <iostream>
#include <algorithm>
#include "../tcp_client.hpp"

class TimeUntilStartHandler {
public:
    static void handle(const char* payload, size_t size, const TCPClient * client) {
        if (size < sizeof(uint32_t)) return;

        uint32_t seconds;
        std::memcpy(&seconds, payload, sizeof(uint32_t));

        client->localTimeLeft = static_cast<int>(seconds);
    }
};
