#pragma once
#include "laps.hpp"

class LapsUpdateHandler {
public:
    static void handle(const PacketBuffer &payload, const ssize_t size) {
        if (size != sizeof(uint16_t) + sizeof(uint8_t))
            throw DeserializationError("Received LapsUpdatePacket with invalid size");

        uint16_t opponentId;
        uint8_t laps;

        std::memcpy(&opponentId, payload.get(), sizeof(opponentId));
        std::memcpy(&laps, payload.get() + sizeof(opponentId), sizeof(laps));

        Laps::getInstance().setOpponentLaps(opponentId, laps);
    }
};
