#pragma once

#include "netcode/client/tcp_client.hpp"

#include <iostream>
#include <atomic>

class NameAcceptedButInQueueHandler {
public:
    static void handle(const TCPClient * client) {
        {
            std::lock_guard<std::mutex> lock(client->lobbyMtx);
            client->lobbyNicks.push_back(client->localNick);
        }

        //client->inLobby.store(true);
        std::cout << "\nYour nickname '" << client->localNick << "' was accepted.\n";
        std::cout<<"\nRace will end soon, stay in the queue!\n";
    }
};
