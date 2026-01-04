#pragma once
#include "../client_handle.hpp"
#include "../tcp_server.hpp"
#include "../../shared/packets/tcp/server/race_end_countdown_packet.hpp"

class LapCountHandler {
public:
    static void handle(const PacketBuffer &payload, const size_t size, ClientHandle &client, TCPServer *server) {
        if (size != sizeof(uint8_t))
            throw DeserializationError("Received LapCountPacket with invalid size");

        const uint8_t laps = static_cast<uint8_t>(*payload.get());
        client.laps = laps;

        server->broadcastLapsUpdate(client);



        // Trigger race end countdown if someone reaches 5 laps
        if (laps >= 1 && !server->raceEndCountdownActive.exchange(true)) {
            server->raceEndStartTime = std::chrono::steady_clock::now();

            RaceEndCountdownPacket packet{};
            server->sendToAllInGame(TCPPacket::serialize(packet), sizeof(packet));

            std::thread([server]() {
                while (true) {
                    const auto now = std::chrono::steady_clock::now();
                    int elapsed = std::chrono::duration_cast<std::chrono::seconds>(now - server->raceEndStartTime).count();
                    int remaining = RACE_END_TIMEOUT - elapsed;

                    if (remaining <= 0) break;

                    std::this_thread::sleep_for(std::chrono::seconds(1));
                }

                // Reset lobby once countdown is finished
                server->raceEndCountdownActive.store(false);
                server->state->endMatch();
            }).detach();
        }
    }
};


