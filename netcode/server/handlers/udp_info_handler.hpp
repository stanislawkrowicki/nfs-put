#pragma once
#include "../client_handle.hpp"
#include "../tcp_server.hpp"

class UdpInfoHandler {
public:
    static void handle(const PacketBuffer &payload, ClientHandle &client,
                       const std::shared_ptr<ClientManager> &clientManager) {
        uint16_t port;
        std::memcpy(&port, payload.get(), sizeof(port));

        sockaddr_in udpAddr{};
        socklen_t addrLen = sizeof(udpAddr);

        if (getpeername(client.tcpSocketFd, reinterpret_cast<sockaddr *>(&udpAddr), &addrLen) == -1) {
            perror("getpeername failed when trying to construct UDP address");
            return;
        }

        std::cout << "Received port: " << port << std::endl;
        udpAddr.sin_port = htons(port);

        clientManager->updateClientUdpAddr(client, udpAddr);
    }
};
