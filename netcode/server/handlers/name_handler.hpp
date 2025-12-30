#pragma once
#include <iostream>
#include <ranges>

#include "../client_handle.hpp"
#include "../tcp_server.hpp"
#include "../../shared/packets/tcp/server/name_accepted_packet.hpp"
#include "../../shared/packets/tcp/server/name_taken_packet.hpp"

class NameHandler {
public:
    static void handle(const std::string &nickname, ClientHandle &client, const TCPServer *server) {
        if (client.state != ClientStateLobby::WaitingForNick) return;

        bool taken = false;
        for (auto &val: server->clientManager->getAllClients() | std::views::values) {
            if (val.nick == nickname && val.id != client.id) {
                taken = true;
                break;
            }
        }

        if (taken) {
            constexpr auto response = NameTakenPacket();
            TCPServer::send(client, TCPPacket::serialize(response), sizeof(response));
        } else {
            client.nick = nickname;
            client.state = ClientStateLobby::InLobby;

            constexpr auto response = NameAcceptedPacket();
            TCPServer::send(client, TCPPacket::serialize(response), sizeof(response));
            std::cout << "\nClient fd=" << client.tcpSocketFd << " set nick: " << nickname << "\n";
            // server->broadcastPlayers();
        }
    }
};
