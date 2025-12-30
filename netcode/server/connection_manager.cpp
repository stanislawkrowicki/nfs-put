#include "connection_manager.hpp"
#include <sys/socket.h>
#include <netdb.h>
#include <stdexcept>
#include <cstring>
#include <iostream>

ConnectionManager &ConnectionManager::getInstance() {
    static ConnectionManager instance;
    return instance;
}

void ConnectionManager::setServerFd(const int fd) {
    serverFd = fd;
}

void ConnectionManager::listen() const {
    if (::listen(serverFd, 30))
        throw std::runtime_error(
            std::string("listen failed! ") + strerror(errno));
}

void ConnectionManager::accept() {
    sockaddr_in clientAddr{};
    socklen_t clientAddrSize = sizeof(clientAddr);

    const auto clientFd = ::accept(serverFd, reinterpret_cast<sockaddr *>(&clientAddr), &clientAddrSize);
    if (clientFd == -1) {
        throw std::runtime_error(std::string("accept failed! ") + strerror(errno));
    }

    ClientHandle client{};
    client.tcpSocketFd = clientFd;
    client.udpAddr = clientAddr;
    client.id = lastClientId;

    clients[client.id] = client;

    lastClientId++;

    char host[NI_MAXHOST], port[NI_MAXSERV];
    getnameinfo(reinterpret_cast<sockaddr *>(&clientAddr), clientAddrSize, host, NI_MAXHOST, port, NI_MAXSERV, 0);
    printf("new connection from: %s:%s (fd: %d)\n", host, port, clientFd);
}
