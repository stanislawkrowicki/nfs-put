#include <iostream>
#include <sys/socket.h>
#include <netdb.h>
#include <cstring>
#include <thread>

#include "loop.hpp"
#include "udp_server.hpp"

int main(const int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <port>", argv[0]);
        return 1;
    }

    const auto clientManager = std::make_shared<ClientManager>();

    const auto udpServer = std::make_unique<UDPServer>(clientManager);

    std::thread udpServerThread([&] {
        udpServer->listen(argv[1]);
    });

    std::thread gameLoopThread([&] {
        new Loop(*udpServer);
    });

    udpServerThread.join();
    gameLoopThread.detach();

    return 0;
}
