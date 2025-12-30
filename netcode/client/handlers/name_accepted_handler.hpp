#pragma once

#include <iostream>

class NameAcceptedHandler {
public:
    static void handle(const TCPClient * client) {
        {
            std::lock_guard<std::mutex> lock(client->lobbyMtx);
            client->lobbyNicks.push_back(client->localNick);
        }

        std::cout << "\nYour nickname '" << client->localNick << "' was accepted.\n";
    }
};
