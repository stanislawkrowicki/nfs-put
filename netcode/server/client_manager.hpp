#pragma once
#include <unordered_map>
#include <netdb.h>

#include "client_handle.hpp"

#include <ranges>
#include <bits/stl_vector.h>

class ClientManager {
public:
    int numberOfConnectedClients = 0;
    ClientHandle *getClient(const uint16_t id) {
        const auto client = clients.find(id);
        if (client == clients.end()) return nullptr;

        return &client->second;
    }
    void resetAll() {
        for (auto& [id, client] : clients) {
            if (client.connected) {
                close(client.tcpSocketFd);
            }
        }
        clients.clear();
        numberOfConnectedClients=0;
    }

    bool nameTaken(const std::string & nickname, const uint16_t client_id) {
        for (auto &val: getAllClients() | std::views::values) {
            if (val.nick == nickname && val.id != client_id)
                return true;
        }
        return false;
    }

    ClientHandle *getClient(const sockaddr_in &address) {
        const auto clientAddress = clientIdsByAddress.find(packAddress(address));
        if (clientAddress == clientIdsByAddress.end()) {
            return nullptr;
        }

        const auto client = clients.find(clientAddress->second);
        if (client == clients.end()) {
            return nullptr;
        }

        return &client->second;
    }

    std::vector<std::string> getNicksInLobby() {
        std::vector<std::string> nicks;
        for (const auto &[id, c] : getAllClients()) {
            if (c.state == ClientStateLobby::InLobby)
                nicks.push_back(c.nick);
        }
        return nicks;
    }

    ClientHandle* getClientByFd(int fd) {
        for (auto &[id, client] : clients) {
            if (client.tcpSocketFd == fd)
                return &client;
        }
        return nullptr;
    }

    void updateClientUdpAddr(ClientHandle &client, const sockaddr_in udpAddr) {
        clientIdsByAddress.erase(packAddress(client.udpAddr));
        client.udpAddr = udpAddr;
        clientIdsByAddress.emplace(packAddress(client.udpAddr), client.id);
    }

    void removeClient(int fd) {
        for (auto it = clients.begin(); it != clients.end(); ++it) {
            if (it->second.tcpSocketFd == fd) {
                clientIdsByAddress.erase(packAddress(it->second.udpAddr));

                close(it->second.tcpSocketFd);

                clients.erase(it);
                numberOfConnectedClients--;
                break;
            }
        }
    }
    void setNickName(const int fd, const std::string &nickname) {
        ClientHandle* client = getClientByFd(fd);
        if (!client) return;
        client->nick = nickname;
    }

    int getNumberOfConnectedClients() const {
        return numberOfConnectedClients;
    }

    std::unordered_map<uint16_t, ClientHandle> &getAllClients() { return clients; }

    void ToLobby(const std::string &nickname, ClientHandle & client) {
        client.nick = nickname;
        client.state = ClientStateLobby::InLobby;
        numberOfConnectedClients++;
    }

    ClientHandle *newClient(sockaddr_in addr, int fd) {
        auto client = ClientHandle();
        client.udpAddr = addr;
        client.id = lastClientId;

        client.tcpSocketFd = fd;

        client.connected = true;

        client.connected = true;

        client.gridPosition = lastClientId;

        clients.emplace(lastClientId, client);
        clientIdsByAddress.emplace(packAddress(client.udpAddr), lastClientId);

        lastClientId++;

        char host[NI_MAXHOST], port[NI_MAXSERV];
        getnameinfo(reinterpret_cast<sockaddr *>(&addr), sizeof(addr), host, NI_MAXHOST, port, NI_MAXSERV, 0);
        printf("new connection from: %s:%s\n", host, port);

        return &clients[lastClientId - 1];
    };

private:
    static uint64_t packAddress(const sockaddr_in &addr) {
        return (static_cast<uint64_t>(addr.sin_addr.s_addr) << 16) |
               ntohs(addr.sin_port);
    }

    // TODO: Maybe ClientHandle should be stored as a shared_ptr
    // so that we can avoid complications when client is removed while doing something on their object
    std::unordered_map<uint16_t, ClientHandle> clients;
    std::unordered_map<uint64_t, uint16_t> clientIdsByAddress;
    uint16_t lastClientId = 0;

};
