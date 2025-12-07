#include "loop.hpp"

#include <chrono>
#include <iostream>
#include <thread>
#include <format>

using namespace std::chrono;

Loop::Loop(const UDPServer &server) : server(server), exit(false) {
    const auto tickDuration = milliseconds(1000 / TICK_RATE);
    int tickCounter = 0;
    auto nextTick = steady_clock::now();

    while (!exit) {
        nextTick = nextTick + tickDuration;

        sendMessageToAll();

        if (tickCounter % 100 == 0) {
            std::cout << std::format("Finished tick {}, time until next tick is {}", tickCounter,
                                     nextTick - steady_clock::now())
                    << std::endl;
        }

        tickCounter++;
        std::this_thread::sleep_until(nextTick);
    }
}

void Loop::sendMessageToAll() const {
    constexpr char message[] = "Hello this is a tick!\n";
    server.sendToAll(message, sizeof(message));
}
