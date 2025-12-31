#pragma once

#include <iostream>
#include <atomic>

class NameAcceptedHandler {
public:
    static void handle(const TCPClient * client) {
        {
            std::lock_guard<std::mutex> lock(client->lobbyMtx);
            client->lobbyNicks.push_back(client->localNick);
        }

        std::cout << "\nYour nickname '" << client->localNick << "' was accepted.\n";
        client->inLobby.store(true);
    }
};
