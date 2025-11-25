#include <iostream>
#include <sys/socket.h>
#include <netdb.h>
#include <cstring>

#include "connection_manager.hpp"

int main(const int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <port>", argv[0]);
        return 1;
    }

    addrinfo *res, hints{};
    hints.ai_socktype = SOCK_STREAM;

    if (const int rv = getaddrinfo(nullptr, argv[1], &hints, &res)) {
        fprintf(stderr, "getaddrinfo error: %s", gai_strerror(rv));
        return 1;
    }

    const int serverFd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
    if (serverFd == -1) {
        std::cerr << "Socket failed: " << strerror(errno) << std::endl;
        return 1;
    }

    constexpr int one = 1;
    setsockopt(serverFd, SOL_SOCKET, SO_REUSEADDR, &one, sizeof(one));

    if (bind(serverFd, res->ai_addr, res->ai_addrlen)) {
        std::cerr << "Bind failed: " << strerror(errno) << std::endl;
        return 1;
    }

    ConnectionManager &connectionManager = ConnectionManager::getInstance();
    connectionManager.setServerFd(serverFd);
    connectionManager.listen();

    std::cout << "Waiting for connection..." << std::endl;
    connectionManager.accept();

    std::cout << "Hello server!" << std::endl;
    return 0;
}
