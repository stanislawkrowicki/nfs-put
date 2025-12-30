#pragma once
#include <vector>
#include <string>
#include <cstring>

#include "../tcp_client.hpp"

class LobbyClientListHandler {
public:
    static void handle(const char* payload, size_t size, const TCPClient* client) {
        std::lock_guard<std::mutex> lock(client->lobbyMtx);

        //client->lobbyNicks.clear();

        size_t offset = 0;
        while (offset < size) {
            const char* nick = payload + offset;
            size_t len = std::strlen(nick);

            if (len == 0) break;

            client->lobbyNicks.emplace_back(nick);
            offset += len + 1;
        }
    }
};
