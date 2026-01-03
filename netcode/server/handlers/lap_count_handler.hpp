#pragma once

class LapCountHandler {
public:
    static void handle(const PacketBuffer &payload, const size_t size, ClientHandle &client, const TCPServer *server) {
        if (size != sizeof(uint8_t))
            throw DeserializationError("Received LapCountPacket with invalid size");

        const uint8_t laps = static_cast<uint8_t>(*payload.get());
        client.laps = laps;

        server->broadcastLapsUpdate(client);
    }
};
