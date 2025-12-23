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
    auto state = std::make_shared<ServerState>();
    const auto tcpServer = std::make_shared<TCPServer>(clientManager, state);

    std::thread tcpServerThread([&] {
        tcpServer->listen(argv[1]);
    });
    {
        std::unique_lock<std::mutex> lock(state->mtx);
        state->cv.wait(lock, [&] { return state->udpReady; });
    }
    const auto udpServer = std::make_shared<UDPServer>(clientManager);

    std::thread udpServerThread([&] {
        udpServer->listen(argv[1]);
    });

    std::thread gameLoopThread([&] {
        Loop::run(udpServer);
    });

    udpServerThread.join();
    gameLoopThread.join();

    return 0;
}
