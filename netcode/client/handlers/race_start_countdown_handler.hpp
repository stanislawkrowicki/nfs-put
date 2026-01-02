#pragma once

class RaceStartCountdownHandler {
public:
    static void handle(const PacketBuffer &buf, const ssize_t size, TCPClient *tcpClient) {
        if (size != sizeof(uint8_t))
            throw DeserializationError("Received RaceStartCountdown packet with invalid payload!");

        const auto secondsUntilStart = static_cast<uint8_t>(*buf.get());
        const auto raceStartTime = std::chrono::steady_clock::now() + std::chrono::seconds(secondsUntilStart);

        tcpClient->setRaceStartTime(raceStartTime);
    }
};
