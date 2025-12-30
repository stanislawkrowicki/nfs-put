#pragma once
#include <iostream>
#include <ranges>

#include "../client_handle.hpp"
#include "../tcp_server.hpp"
#include "../../shared/packets/tcp/server/name_accepted_packet.hpp"
#include "../../shared/packets/tcp/server/name_taken_packet.hpp"
#include "../../shared/packets/tcp/server/client_connected_packet.hpp"
#include "../../shared/packets/tcp/server/time_until_start_packet.hpp"
#include "../../shared/packets/tcp/server/lobby_client_list_packet.hpp"

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
            ClientConnectedPacket connected{};
            std::memcpy(connected.payload,
                        client.nick.c_str(),
                        std::min(client.nick.size(), static_cast<size_t>(CONNNAME_PAYLOAD_SIZE)));

            const auto buf = TCPPacket::serialize(connected);

            server->sendToAllInLobbyExcept(buf,
                                    sizeof(ClientConnectedPacket),
                                    client);

            TimeUntilStartPacket countdown{};
            countdown.seconds = server->timeUntilStart();
            auto countdownBuf = TCPPacket::serialize(countdown);
            server->sendToAllInLobby(countdownBuf, sizeof(countdown));

            std::vector<std::string> nicks;
            for (const auto &[id, c] : server->clientManager->getAllClients()) {
                if (c.state == ClientStateLobby::InLobby)
                    nicks.push_back(c.nick);
            }

            LobbyClientListPacket lobbyList(nicks, client.nick);
            auto listBuf = TCPPacket::serialize(lobbyList);
            TCPServer::send(client, listBuf, sizeof(TCPPacketHeader) + lobbyList.header.payloadSize);


            std::cout << "\nClient fd=" << client.socketFd << " set nick: " << nickname << "\n";
            // server->broadcastPlayers();

        }
    }
};
