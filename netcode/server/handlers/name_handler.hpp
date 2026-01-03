#pragma once
#include <iostream>
#include <ranges>

#include "../client_handle.hpp"
#include "../tcp_server.hpp"
#include "../../shared/packets/tcp/client/name_packet.hpp"
#include "../../shared/packets/tcp/server/name_accepted_packet.hpp"
#include "../../shared/packets/tcp/server/name_taken_packet.hpp"
#include "../../shared/packets/tcp/server/client_connected_packet.hpp"
#include "../../shared/packets/tcp/server/time_until_start_packet.hpp"
#include "../../shared/packets/tcp/server/lobby_client_list_packet.hpp"

class NameHandler {
public:
    static void handle(const std::string &nickname, ClientHandle &client, TCPServer *server) {
        if (client.state != ClientStateLobby::WaitingForNick) return;
        if (nickname.size() > MAX_NAME_PAYLOAD_SIZE) return;

        for (const auto ch: nickname)
            if (ch < 32 || ch > 126)
                return;

        if (server->clientManager->nameTaken(nickname, client.id)) {
            sendNameTaken(client);
            return;
        }

        sendNameAcceptedPacket(nickname,client,server);

        std::cout << "\nClient fd=" << client.tcpSocketFd << " set nick: " << nickname << "\n";

        sendClientConnectedPacket(client, server);
        sendTimeUntilStartPacket(server);
        sendClientList(client,server);
    }

    static void sendNameTaken(const ClientHandle & client) {
        constexpr auto response = NameTakenPacket();
        TCPServer::send(client, TCPPacket::serialize(response), sizeof(response));
    }

    static void sendNameAcceptedPacket(const std::string &nickname, ClientHandle &client, TCPServer *server) {
        server->clientManager->ToLobby(nickname, client);
        if (server->clientManager->getNumberOfConnectedClients() == 1) {
            std::lock_guard lock(server->state->mtx);
            if (server->state->phase == MatchPhase::Lobby) {
                server->resetLobbyStartTime();
                server->countdownToLobbyEnd();
            }
        }

        constexpr auto response = NameAcceptedPacket();
        TCPServer::send(client, TCPPacket::serialize(response), sizeof(response));
    }

    static void sendClientConnectedPacket(const ClientHandle &client, const TCPServer *server) {
        const auto [clientConnectedPacket, clientConnectedPacketSize] = TCPPacket::create<ClientConnectedPacket>(
       client.nick.c_str(), client.nick.size());

        const auto buf = TCPPacket::serialize(clientConnectedPacket, clientConnectedPacketSize);

        server->sendToAllInLobbyExcept(buf,
                                       clientConnectedPacketSize,
                                       client);
    }

    static void sendTimeUntilStartPacket(const TCPServer *server) {
        TimeUntilStartPacket countdown{};
        countdown.seconds = server->timeUntilStart();
        auto countdownBuf = TCPPacket::serialize(countdown);

        server->sendToAllInLobby(countdownBuf, sizeof(countdown));
    }

    static void sendClientList(const ClientHandle &client, const TCPServer *server) {
        std::vector<std::string> nicks = server->clientManager->getNicksInLobby();

        if (server->clientManager->getNumberOfConnectedClients()!=1) {
            LobbyClientListPacket lobbyList(nicks, client.nick);

            size_t totalSize = sizeof(TCPPacketHeader) + lobbyList.header.payloadSize;
            auto listBuf = std::make_unique<char[]>(totalSize);

            std::memcpy(listBuf.get(), &lobbyList.header, sizeof(TCPPacketHeader));
            std::memcpy(listBuf.get() + sizeof(TCPPacketHeader), lobbyList.payload.get(), lobbyList.header.payloadSize);

            TCPServer::send(client, listBuf.get(), totalSize);
        }
    }
};
