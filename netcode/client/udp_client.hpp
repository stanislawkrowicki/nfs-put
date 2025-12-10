#pragma once

#define SERVER_IP "127.0.0.1"
#define SERVER_PORT "1313"
#include "../shared/packets/udp/udp_packet.hpp"
#include "LinearMath/btTransform.h"

class UDPClient {
    int socketFd = -1;
    long lastPacketId = 0;

public:
    explicit UDPClient();

    ~UDPClient();

    void send(const char *data, ssize_t size) const;

    void send(const PacketBuffer &data, ssize_t size) const;

    void sendStartMessage() const;

    void sendPosition(const btTransform &transform);
};
