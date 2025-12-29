#pragma once
#include <iostream>

#include "netcode/shared/packets/tcp/tcp_packet.hpp"

class ProvideNameHandler {
public:
    static void handle() {
        std::cout << "Welcome! Please provide your nickname: (up to 32 characters): " << std::flush;
    }
};
