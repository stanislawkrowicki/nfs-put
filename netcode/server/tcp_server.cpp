#include "tcp_server.hpp"
#include <arpa/inet.h>
#include <unistd.h>
#include <fcntl.h>
#include <cstring>
#include <sys/epoll.h>
#include <iostream>
#include <utility>
#include <netdb.h>
#include <algorithm>
#include <thread>
#include <condition_variable>

static int makeNonBlocking(int fd) {
    int flags = fcntl(fd, F_GETFL, 0);
    return fcntl(fd, F_SETFL, flags | O_NONBLOCK);
}

TCPServer::TCPServer(std::shared_ptr<ClientManager> clientManager, std::shared_ptr<ServerState> state) {
    socketFd = ::socket(AF_INET, SOCK_STREAM, 0);
    if (socketFd < 0)
        throw std::runtime_error(std::string("Failed to create TcpBSDServer socket! ") + std::strerror(errno));

    constexpr int one = 1;
    setsockopt(socketFd, SOL_SOCKET, SO_REUSEADDR, &one, sizeof(one));

    makeNonBlocking(socketFd);
    raceStartTime = std::chrono::steady_clock::now();
    this->clientManager = std::move(clientManager);
    this->state = std::move(state);
};

TCPServer::~TCPServer() {
    if (socketFd >= 0)
        close(socketFd);
}
int TCPServer::timeLeft() const {
    auto now = std::chrono::steady_clock::now();
    auto elapsed = std::chrono::duration_cast<std::chrono::seconds>(now - raceStartTime).count();
    int remaining = raceDuration - static_cast<int>(elapsed);
    return remaining > 0 ? remaining : 0;
}

void TCPServer::startCountdown() const {
    std::thread([this]() {
        while (true) {
            int remaining = timeLeft();
            std::cout << "\rRace starts in: " << remaining << "s" << std::flush;

            if (remaining <= 0) {
                std::cout << "\nRace started!\n";
                {
                    std::lock_guard<std::mutex> lock(state->mtx);
                    state->udpReady = true;
                }
                state->cv.notify_all();
                const auto& clients = clientManager->getAllClients();
                std::string raceBegin="RACE BEGINS! - said the TCP server";
                for (const auto& [id, client] : clients) {
                    if (!client.connected || client.state != ClientStateLobby::InLobby) continue;
                    send(client, raceBegin.c_str(), raceBegin.size());
                }
                break;
            }

            std::this_thread::sleep_for(std::chrono::seconds(1));
        }
    }).detach();
}

void TCPServer::listen(const char *port) {
    addrinfo *res, hints{};
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_family = AF_INET;

    if (const int rv = getaddrinfo(nullptr, port, &hints, &res))
        throw std::runtime_error(std::string("TcpBSDServer getaddrinfo failed: ") + gai_strerror(rv));

    if (::bind(socketFd, res->ai_addr, res->ai_addrlen))
        throw std::runtime_error(std::string("TcpBSDServer bind failed: ") + strerror(errno));

    if (::listen(socketFd, SOMAXCONN))
        throw std::runtime_error(std::string("TcpBSDServer listen failed: ") + strerror(errno));
    freeaddrinfo(res);
    startCountdown();
    loop();
}


void TCPServer::send(const ClientHandle client, const char *data, const ssize_t size) const {
    if (!client.connected) {
        std::cout << "Tried to send to not connected" << std::endl;
        return;
    }

    const ssize_t bytesSent = ::send(client.socketFd, data, size, 0);

    if (bytesSent <= 0)
        throw std::runtime_error(std::string("Failed to TCP message: ") + strerror(errno));
}

[[noreturn]]
void TCPServer::loop() const {
    int efd = epoll_create1(0);
    if (efd < 0)
        throw std::runtime_error(std::string("epoll_create1 failed: ") + strerror(errno));
    epoll_event ev{};
    ev.events = EPOLLIN;
    ev.data.fd = socketFd;
    epoll_ctl(efd, EPOLL_CTL_ADD, socketFd, &ev);
    std::cout<<"waiting for clients...\n";
    epoll_event events[64];
    while (true) {
        int n = epoll_wait(efd, events, 64, -1);
        if (n < 0)
            throw std::runtime_error(std::string("epoll_wait failed: ") + strerror(errno));
        for (int i = 0; i < n; ++i) {
            int fd = events[i].data.fd;
            if (fd == socketFd) {
                while (true) {
                    sockaddr_in cli{};
                    socklen_t clilen = sizeof(cli);
                    int cfd = accept(socketFd, reinterpret_cast<sockaddr*>(&cli), &clilen);
                    if (cfd < 0) break;
                    makeNonBlocking(cfd);
                    auto *client =clientManager->newClient(cli,cfd);
                    client->state = ClientStateLobby::WaitingForNick;
                    const char *message="Welcome to the race! Please provide us with your unique nickname! (max 20 characters)\n";
                    size_t messageLen = strlen(message);
                    send(*client, message, messageLen);
                    epoll_event cev{};
                    cev.events = EPOLLIN | EPOLLRDHUP;
                    cev.data.fd = cfd;
                    epoll_ctl(efd, EPOLL_CTL_ADD, cfd, &cev);
                    std::cout<<"Accepted client fd="<<cfd<<"\n";
                }
                continue;
            }
            if (events[i].events & (EPOLLRDHUP | EPOLLHUP | EPOLLERR)) {
                clientManager->removeClient(fd);
                std::cout<<"Client disconnected fd="<<fd<<"\n";
                broadcastPlayers();
                continue;
            }
            if (events[i].events & EPOLLIN) {
                char buf[21];
                ssize_t bytesRead = recv(fd, buf, sizeof(buf), 0);
                if (bytesRead <= 0) {
                    clientManager->removeClient(fd);
                    std::cout<<"Client disconnected fd="<<fd<<"\n";
                    broadcastPlayers();
                    continue;
                }
                ClientHandle *client = clientManager->getClientByFd(fd);
                if (!client) continue;
                Packet packet{
                    .data = std::make_unique<char[]>(bytesRead),
                    .size = bytesRead,
                    .sender = client
                };
                memcpy(packet.data.get(), buf, bytesRead);

                parsePacket(packet);
            }
        }
    }
}

void TCPServer::parsePacket(const Packet &packet) const {
    ClientHandle *client = packet.sender;
    if (client->state == ClientStateLobby::WaitingForNick) {
        handleNick(packet);
    } else {
        std::cout<<"not nick data from a client:\n";
        write(STDOUT_FILENO, packet.data.get(), packet.size);
    }
}
void TCPServer::handleNick(const Packet &packet) const {
    ClientHandle *client = packet.sender;

    std::string nick(packet.data.get(), packet.size);
    nick.erase(std::remove(nick.begin(), nick.end(), '\n'), nick.end());
    nick.erase(std::remove(nick.begin(), nick.end(), '\r'), nick.end());

    bool taken = false;
    for (auto &p : clientManager->getAllClients()) {
        if (p.second.nick == nick && p.second.id != client->id) {
            taken = true;
            break;
        }
    }
    const char *message;
    if (taken) {
        message = "Nick taken. Choose another one.\n";
        size_t messageLen = strlen(message);
        send(*client, message, messageLen);
    } else {
        client->nick = nick;
        client->state = ClientStateLobby::InLobby;
        message="Nick accepted! Welcome to the lobby.\n";
        size_t messageLen = strlen(message);
        send(*client, message, messageLen);
        std::cout << "Client fd=" << client->socketFd << " set nick: " << nick << "\n";
        broadcastPlayers();
    }
}
void TCPServer::broadcastPlayers() const {
    const auto& clients = clientManager->getAllClients();
    size_t connectedCount = 0;
    for (const auto& [id, client] : clients) {
        if (client.connected && client.state == ClientStateLobby::InLobby)
            ++connectedCount;
    }
    std::string lobbyMessage = "Player Count (" + std::to_string(connectedCount) + "):\n";

    int index = 1;
    for (const auto& [id, client] : clients) {
        if (!client.connected || client.state != ClientStateLobby::InLobby) continue;
        lobbyMessage += std::to_string(index++) + ". " + client.nick + "\n";
    }
    lobbyMessage += "Race starts in: " + std::to_string(timeLeft()) + "s\n";
    for (const auto& [id, client] : clients) {
        if (!client.connected || client.state != ClientStateLobby::InLobby) continue;
        send(client, lobbyMessage.c_str(), lobbyMessage.size());
    }
}

