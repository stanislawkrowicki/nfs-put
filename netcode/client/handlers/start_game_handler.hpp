#pragma once

class StartGameHandler {
public:
    static void handle(const PacketBuffer &payload, const ssize_t size, TCPClient *client) {
        if (size != sizeof(uint8_t))
            throw DeserializationError("Received StartGame packet with invalid payload size");

        const auto gridPosition = static_cast<uint8_t>(*payload.get());
        client->setGridPosition(gridPosition);

        client->setGameReady();
    }
};
