#include "client.hpp"
#include <arpa/inet.h>
#include <unistd.h>
#include <cstring>
#include <format>
#include <iostream>
#include <utility>
#include <netdb.h>

#include "netcode/shared/packets/udp/position_packet.hpp"

Client::Client() {
    addrinfo hints{};
    addrinfo *result{};

    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_DGRAM;
    hints.ai_protocol = IPPROTO_UDP;

    const int status = getaddrinfo(SERVER_IP, SERVER_PORT, &hints, &result);
    if (status != 0)
        throw std::runtime_error(std::format("getaddrinfo failed: {}", gai_strerror(status)));

    bool connectedSuccessfully = false;

    for (; result != nullptr; result = result->ai_next) {
        socketFd = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
        if (socketFd < 0)
            continue;

        if (connect(socketFd, result->ai_addr, result->ai_addrlen) == 0) {
            std::cout << "Connected to the server!" << std::endl;
            connectedSuccessfully = true;
            break;
        }
    }

    freeaddrinfo(result);

    if (!connectedSuccessfully) {
        ::close(socketFd);
        socketFd = -1;
        throw std::runtime_error(std::format("Failed to connect to the server with IP {} and port {}.", SERVER_IP,
                                             SERVER_PORT));
    }
}

Client::~Client() {
    if (socketFd >= 0)
        close(socketFd);
}

void Client::send(const char *data, const ssize_t size) const {
    write(socketFd, data, size);
}

void Client::send(const PacketBuffer &data, const ssize_t size) const {
    write(socketFd, data.get(), size);
}

void Client::sendStartMessage() const {
    constexpr char message[] = "1";
    send(message, 1);
}

void Client::sendPosition(const btTransform &transform) {
    btTransformFloatData floatData{};
    transform.serialize(floatData);

    const auto packet = UDPPacket::create<PositionPacket>(13, lastPacketId, reinterpret_cast<const char *>(&floatData),
                                                          64);

    send(UDPPacket::serialize(packet), sizeof(packet));
    lastPacketId++;
}
