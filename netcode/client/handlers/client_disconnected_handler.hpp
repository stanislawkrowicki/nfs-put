#pragma once
#include <algorithm>
#include "../tcp_client.hpp"

class ClientDisconnectedHandler {
public:
    static void handle(const char* payload, size_t size, const TCPClient * client) {

      {
          const std::string nick(payload, strnlen(payload, size));
          std::lock_guard<std::mutex> lock(client->lobbyMtx);
            if (const auto it = std::ranges::find(client->lobbyNicks, nick); it != client->lobbyNicks.end())
                client->lobbyNicks.erase(it);
        }
    }
};
