#include "loop.hpp"

#include <chrono>
#include <iostream>
#include <thread>
#include <format>
#include <ranges>

using namespace std::chrono;

std::shared_ptr<UDPServer> Loop::server;
bool Loop::exit = false;
std::vector<ClientState> Loop::statesToUpdate{};

void Loop::run(const std::shared_ptr<UDPServer> &udpServer) {
    server = udpServer;
    exit = false;

    const auto tickDuration = milliseconds(1000 / TICK_RATE);
    int tickCounter = 0;
    auto nextTick = steady_clock::now();

    while (!exit) {
        nextTick = nextTick + tickDuration;

        if (!statesToUpdate.empty()) {
            sendStates();
        }

        if (tickCounter % 100 == 0) {
            std::cout << std::format("Finished tick {}, time until next tick is {}", tickCounter,
                                     nextTick - steady_clock::now())
                    << std::endl;
        }

        tickCounter++;
        std::this_thread::sleep_until(nextTick);
    }
}

void Loop::enqueueStateUpdate(const ClientState &state) {
    statesToUpdate.push_back(state);
}

void Loop::sendMessageToAll() {
    constexpr char message[] = "Hello this is a tick!\n";
    server->sendToAll(message, sizeof(message));
}

/* TODO: Make this thread safe (statesToUpdate can be updated while this function is executing) */
void Loop::sendStates() {
    constexpr int POSITIONS_PER_PACKET = 5;

    std::vector<OpponentStatesPacket> packets;

    for (long i = 0; i < statesToUpdate.size(); i += POSITIONS_PER_PACKET) {
        const long currentBatchSize = std::min(POSITIONS_PER_PACKET, static_cast<int>(statesToUpdate.size() - i));

        std::vector batch(statesToUpdate.begin() + i,
                          statesToUpdate.begin() + i + currentBatchSize);

        packets.push_back(packStatesBatch(batch));
    }

    statesToUpdate.clear();

    for (const auto &packet: packets) {
        auto buf = serializeOpponentState(packet);
        /* TODO: Player should not get their own positions */
        server->sendToAll(buf.get(), static_cast<ssize_t>(getOpponentStatePacketSize(packet)));
    }
}

OpponentStatesPacket Loop::packStatesBatch(const std::vector<ClientState> &batch) {
    OpponentStatesPacket packet;

    packet.header = {
        .type = UDPPacketType::PositionResponse,
        .id = 0,
    };

    packet.statesCount = batch.size();
    packet.states = batch;

    // TODO: Add missing checksum
    return packet;
}
