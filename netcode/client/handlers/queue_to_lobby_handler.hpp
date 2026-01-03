#pragma once

#include "netcode/client/tcp_client.hpp"

#include <iostream>
#include <atomic>

class QueueToLobbyHandler {
public:
    static void handle(const TCPClient * client) {
        client->inLobby.store(true);
        std::cout << "\nWelcome to the lobby\n";
    }
};
