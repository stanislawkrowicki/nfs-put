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
        size_t remaining = size - offset;

        // find '\0' within bounds
        const char* end = static_cast<const char*>(
            memchr(payload + offset, '\0', remaining)
        );

        if (!end) break; // malformed packet

        size_t len = end - (payload + offset);
        if (len == 0) break;

        client->lobbyNicks.emplace_back(payload + offset, len);
        offset += len + 1;
    }
}

};
