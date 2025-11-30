#pragma once
#include <unordered_map>
#include <netdb.h>

#include "client_handle.hpp"

class ClientManager {
public:
    ClientHandle *getClient(const uint16_t id) {
        const auto client = clients.find(id);
        if (client == clients.end()) return nullptr;

        return &client->second;
    };

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
    };

    std::unordered_map<uint16_t, ClientHandle> &getAllClients() { return clients; };

    ClientHandle *newClient(sockaddr_in addr) {
        auto client = ClientHandle();
        client.address = addr;
        client.id = lastClientId;

        // TODO: set sock on connected to lobby
        client.socketFd = -1;

        client.connected = true;

        clients.emplace(lastClientId, client);
        clientIdsByAddress.emplace(packAddress(client.address), lastClientId);

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
