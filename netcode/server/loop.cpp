#include "loop.hpp"

#include <chrono>
#include <iostream>
#include <thread>
#include <format>
#include <ranges>

using namespace std::chrono;

std::shared_ptr<UDPServer> Loop::server;
bool Loop::exit = false;
std::vector<PlayerPosition> Loop::positionsToUpdate{};

void Loop::run(const std::shared_ptr<UDPServer> &udpServer) {
    server = udpServer;
    exit = false;

    const auto tickDuration = milliseconds(1000 / TICK_RATE);
    int tickCounter = 0;
    auto nextTick = steady_clock::now();

    while (!exit) {
        nextTick = nextTick + tickDuration;

        if (!positionsToUpdate.empty()) {
            sendPositions();
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

void Loop::enqueuePositionUpdate(const PlayerPosition &position) {
    positionsToUpdate.push_back(position);
}

void Loop::sendMessageToAll() {
    constexpr char message[] = "Hello this is a tick!\n";
    server->sendToAll(message, sizeof(message));
}

/* TODO: Make this thread safe (positionsToUpdate can be updated while this function is executing) */
void Loop::sendPositions() {
    constexpr int POSITIONS_PER_PACKET = 5;

    std::vector<OpponentPositionsPacket> packets;

    for (long i = 0; i < positionsToUpdate.size(); i += POSITIONS_PER_PACKET) {
        const long currentBatchSize = std::min(POSITIONS_PER_PACKET, static_cast<int>(positionsToUpdate.size() - i));

        std::vector batch(positionsToUpdate.begin() + i,
                          positionsToUpdate.begin() + i + currentBatchSize);

        packets.push_back(packPositionsBatch(batch));
    }

    positionsToUpdate.clear();

    for (const auto &packet: packets) {
        auto buf = serializeOpponentPositionsPacket(packet);
        /* TODO: Player should not get their own positions */
        server->sendToAll(buf.get(), static_cast<ssize_t>(getOpponentPositionsPacketSize(packet)));
    }
}

OpponentPositionsPacket Loop::packPositionsBatch(const std::vector<PlayerPosition> &batch) {
    OpponentPositionsPacket packet;

    packet.header = {
        .type = UDPPacketType::PositionResponse,
        .id = 0,
    };

    packet.positionsCount = batch.size();
    packet.positions = batch;

    // TODO: Add missing checksum
    return packet;
}
