#include "udp_server.hpp"
#include <arpa/inet.h>
#include <unistd.h>
#include <cstring>
#include <iostream>
#include <utility>
#include <netdb.h>

UDPServer::UDPServer(std::shared_ptr<ClientManager> clientManager) {
    socketFd = ::socket(AF_INET, SOCK_DGRAM, 0);
    if (socketFd < 0)
        throw std::runtime_error(std::string("Failed to create UdpBSDServer socket! ") + std::strerror(errno));

    // constexpr int one = 1;
    // setsockopt(socketFd, SOL_SOCKET, SO_REUSEADDR, &one, sizeof(one));

    this->clientManager = std::move(clientManager);
};

UDPServer::~UDPServer() {
    if (socketFd >= 0)
        close(socketFd);
}

void UDPServer::listen(const char *port) {
    addrinfo *res, hints{};
    hints.ai_socktype = SOCK_DGRAM;
    hints.ai_family = AF_INET;

    if (const int rv = getaddrinfo(nullptr, port, &hints, &res))
        throw std::runtime_error(std::string("UdpBSDServer getaddrinfo failed: ") + gai_strerror(rv));

    if (::bind(socketFd, res->ai_addr, res->ai_addrlen))
        throw std::runtime_error(std::string("UdpBSDServer bind failed: ") + strerror(errno));

    loop();
}

void UDPServer::addMessageListener(const PacketListener listener) {
    listeners.push_back(listener);
}

void UDPServer::send(const ClientHandle client, const char *data, const ssize_t size) const {
    if (!client.connected) {
        std::cout << "Tried to send to not connected" << std::endl;
        return;
    }

    const ssize_t bytesSent = ::sendto(socketFd, data, size, 0, reinterpret_cast<const sockaddr *>(&client.address),
                                       sizeof(client.address));

    if (bytesSent <= 0)
        throw std::runtime_error(std::string("Failed to UDP message: ") + strerror(errno));
}

[[noreturn]]
void UDPServer::loop() const {
    while (true) {
        auto buf = std::make_unique<char[]>(MAX_PACKET_SIZE);

        sockaddr_in sender = {};
        socklen_t senderSize = sizeof(sender);

        const ssize_t bytesRead = recvfrom(socketFd, buf.get(), MAX_PACKET_SIZE, 0,
                                           reinterpret_cast<sockaddr *>(&sender), &senderSize);

        if (bytesRead < 0) {
            perror("recvfrom");
            continue;
        }

        auto client = clientManager->getClient(sender);

        if (!client) {
            client = clientManager->newClient(sender);
        }

        auto packet = Packet{
            .data = std::move(buf),
            .size = bytesRead,
            .sender = client
        };

        parsePacket(packet);
    }
}

void UDPServer::parsePacket(const Packet &packet) const {
    write(STDOUT_FILENO, packet.data.get(), packet.size);
    send(*packet.sender, "Welcome", 7);
}

