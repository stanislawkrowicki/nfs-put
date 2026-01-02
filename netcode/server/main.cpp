#include <iostream>
#include <sys/socket.h>
#include <netdb.h>
#include <cstring>
#include <thread>
#include <condition_variable>

#include "loop.hpp"
#include "tcp_server.hpp"
#include "udp_server.hpp"


int main(const int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <port>", argv[0]);
        return 1;
    }

    const auto clientManager = std::make_shared<ClientManager>();
    const auto udpServer = std::make_shared<UDPServer>(clientManager);

    auto state = std::make_shared<ServerState>();
    const auto tcpServer = std::make_shared<TCPServer>(clientManager, state);

    std::thread udpServerThread([&] {
        udpServer->listen(argv[1]);
    });

    udpServerThread.detach();

    std::thread tcpServerThread([&] {
        tcpServer->listen(argv[1]);
    });
    tcpServerThread.detach();

    while (true) {

        {
            std::unique_lock lock(state->mtx);
            state->cv.wait(lock, [&] {
                return state->phase == MatchPhase::Running;
            });
        }

        std::thread gameThread([&] {
            Loop::run(udpServer, state);
        });

        gameThread.join();

        //tcpServer->notifyMatchEnded();

        tcpServer->resetLobbyStartTime();
        Loop::reset();
        tcpServer->resetLobby();
        clientManager->resetAll();

        {
            std::lock_guard lock(state->mtx);
            state->phase = MatchPhase::Lobby;
        }

        std::cout << "Server reset. Waiting for new clients...\n";
    }

    return 0;
}
