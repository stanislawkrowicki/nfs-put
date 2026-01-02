#include "loop.hpp"

#include <chrono>
#include <iostream>
#include <thread>
#include <format>
#include <ranges>

using namespace std::chrono;

std::shared_ptr<UDPServer> Loop::server;
std::unordered_map<uint16_t, ClientState> Loop::latestClientStates{};

void Loop::reset() {
    latestClientStates.clear();
}
void Loop::run(const std::shared_ptr<UDPServer> &udpServer,const std::shared_ptr<ServerState>& state) {
    server = udpServer;

    const auto tickDuration = milliseconds(1000 / TICK_RATE);
    int tickCounter = 0;
    auto nextTick = steady_clock::now();

    while (true) {
        {
            std::lock_guard lock(state->mtx);
            if (state->phase == MatchPhase::Finished)
                break;
        }
        nextTick = nextTick + tickDuration;

        if (!latestClientStates.empty()) {
            sendLatestStates();
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
    latestClientStates.insert_or_assign(state.clientId, state);
}

/* TODO: Make this thread safe (statesToUpdate can be updated while this function is executing) */
void Loop::sendLatestStates() {
    constexpr int STATES_PER_PACKET = 5;

    for (const auto client: server->getAllClients() | std::views::values) {
        std::vector<OpponentStatesPacket> packets;

        auto opponentStates = latestClientStates
                              | std::views::filter([client](const auto &pair) {
                                  return pair.first != client.id;
                              }) | std::views::values;

        std::vector<ClientState> batch{};
        for (const auto &state: opponentStates) {
            batch.push_back(state);

            if (batch.size() == STATES_PER_PACKET) {
                packets.push_back(packStatesBatch(batch));
                batch.clear();
            }
        }

        if (!batch.empty())
            packets.push_back(packStatesBatch(batch));

        for (const auto &packet: packets) {
            auto buf = serializeOpponentState(packet);
            server->send(client, buf, static_cast<ssize_t>(getOpponentStatePacketSize(packet)));
        }
    }

    latestClientStates.clear();
}

OpponentStatesPacket Loop::packStatesBatch(const std::vector<ClientState> &batch) {
    OpponentStatesPacket packet;

    packet.header = {
        .type = UDPPacketType::OpponentStates,
        .id = 0,
    };

    packet.statesCount = batch.size();
    packet.states = batch;

    const auto serialized = serializeOpponentState(packet);

    const auto packetSize = OPPONENT_STATES_PACKET_SIZE_WITHOUT_DATA + sizeof(ClientState) * packet.statesCount;
    const auto checksum = UDPPacket::calculatePacketChecksum(serialized, packetSize);

    packet.checksum = checksum;
    return packet;
}
