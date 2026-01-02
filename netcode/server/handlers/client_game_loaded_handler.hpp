#pragma once
#include <memory>
#include <ranges>

#include "../client_handle.hpp"
#include "../client_manager.hpp"

class ClientGameLoadedHandler {
public:
    static void handle(ClientHandle &client, const std::shared_ptr<ClientManager> &clientManager, TCPServer *server) {
        client.gameLoaded = true;

        for (const auto &otherClient: clientManager->getAllClients() | std::views::values) {
            if (otherClient.state == ClientStateLobby::InGame && !otherClient.gameLoaded)
                return;
        }

        server->startRaceStartCountdown();
    }
};
