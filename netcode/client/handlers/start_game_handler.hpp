#pragma once
#include "netcode/shared/packets/tcp/server/start_game_packet.hpp"
#include "netcode/shared/packets/tcp/tcp_packet_header.hpp"


struct __attribute__((packed)) StartGamePayload {
    uint8_t gridPosition;
    PlayerVehicleColor vehicleColor;
};

class StartGameHandler {
public:
    static void handle(const PacketBuffer &payload,
                       const ssize_t size,
                       TCPClient *client)
    {
        if (size != sizeof(StartGamePayload))
            throw DeserializationError("Invalid StartGame payload size");

        StartGamePayload data{};
        std::memcpy(&data, payload.get(), sizeof(StartGamePayload));

        client->setGridPosition(data.gridPosition);
        client->setColor(data.vehicleColor);
        client->setGameReady();
    }
};
