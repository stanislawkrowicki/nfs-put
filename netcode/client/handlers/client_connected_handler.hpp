#pragma once
#include <iostream>
#include <algorithm>
#include "../tcp_client.hpp"

class ClientConnectedHandler {
public:
    static void handle(const char* payload, size_t size, const TCPClient * client) {
      const std::string nick(payload, strnlen(payload, size));

        {
            std::lock_guard<std::mutex> lock(client->lobbyMtx);
            if (std::ranges::find(client->lobbyNicks, nick) == client->lobbyNicks.end())
                client->lobbyNicks.push_back(nick);
        }

        //std::cout << "\n" << nick << " joined the lobby\n";
    }
};
