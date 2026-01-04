#pragma once
#include "netcode/client/tcp_client.hpp"

#include <chrono>

class RaceEndCountdownHandler {
public:
    static void handle(const PacketBuffer &buf, const ssize_t size, TCPClient *tcpClient) {
        if (size < sizeof(uint8_t))
            throw DeserializationError("RaceEndCountdown packet too small");

        uint8_t seconds;
        std::memcpy(&seconds, buf.get(), sizeof(seconds));

        tcpClient->setRaceEndSeconds(seconds);
    }
};
