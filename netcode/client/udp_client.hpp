#pragma once

#define SERVER_IP "127.0.0.1"
#define SERVER_PORT "1313"
#include "../shared/packets/udp/udp_packet.hpp"
#include "LinearMath/btTransform.h"

class UDPClient {
    static constexpr int MAX_MESSAGE_SIZE = 512;

    int socketFd = -1;
    long lastPacketId = 0;
    uint16_t clientId{};
    volatile bool waitForMessages = false;

public:
    explicit UDPClient();

    ~UDPClient();

    void send(const char *data, ssize_t size) const;

    void send(const PacketBuffer &data, ssize_t size) const;

    void sendStartMessage() const;

    void sendPosition(const btTransform &transform);

    void handlePacket(const PacketBuffer &buf, ssize_t size) const;

    void listen();

    void stopListening();

    void close();
};
