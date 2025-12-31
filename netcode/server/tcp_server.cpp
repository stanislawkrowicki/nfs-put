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
#include <ranges>

#include "../shared/packets/tcp/server/provide_name_packet.hpp"
#include "../shared/packets/tcp/server/race_start_packet.hpp"
#include "../shared/packets/tcp/server/client_disconnected_packet.hpp"
#include "handlers/name_handler.hpp"
#include "handlers/udp_info_handler.hpp"

static int makeNonBlocking(const int fd) {
    const int flags = fcntl(fd, F_GETFL, 0);
    return fcntl(fd, F_SETFL, flags | O_NONBLOCK);
}

TCPServer::TCPServer(std::shared_ptr<ClientManager> clientManager, std::shared_ptr<ServerState> state) {
    socketFd = ::socket(AF_INET, SOCK_STREAM, 0);
    if (socketFd < 0)
        throw std::runtime_error(std::string("Failed to create TcpBSDServer socket! ") + std::strerror(errno));

    constexpr int one = 1;
    setsockopt(socketFd, SOL_SOCKET, SO_REUSEADDR, &one, sizeof(one));

    makeNonBlocking(socketFd);
    lobbyStartTime = std::chrono::steady_clock::now();
    this->clientManager = std::move(clientManager);
    this->state = std::move(state);
};

TCPServer::~TCPServer() {
    if (socketFd >= 0)
        close(socketFd);
}

int TCPServer::timeUntilStart() const {
    const auto now = std::chrono::steady_clock::now();
    const auto elapsed = std::chrono::duration_cast<std::chrono::seconds>(now - lobbyStartTime).count();
    const int remaining = raceStartTimeout - static_cast<int>(elapsed);
    return remaining > 0 ? remaining : 0;
}

void TCPServer::startCountdown() const {
    std::thread([this]() {
        while (true) {
            const int remaining = timeUntilStart();
            std::cout << "\rRace starts in: " << remaining << "s" << std::flush;

            if (remaining <= 0) {
                std::cout << "\nRace started!\n"; {
                    std::lock_guard<std::mutex> lock(state->mtx);
                    state->udpReady = true;
                }
                state->cv.notify_all();
                const auto &clients = clientManager->getAllClients();

                for (const auto &client: clients | std::views::values) {
                    if (!client.connected || client.state != ClientStateLobby::InLobby) continue;
                    auto packet = RaceStartPacket();
                    const auto serialized = TCPPacket::serialize(packet);
                    send(client, serialized, sizeof(packet));
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


void TCPServer::send(const ClientHandle &client, const char *data, const ssize_t size) {
    if (!client.connected) {
        std::cout << "Tried to send to not connected" << std::endl;
        return;
    }

    const ssize_t bytesSent = ::send(client.tcpSocketFd, data, size, 0);

    if (bytesSent <= 0 && errno != EWOULDBLOCK && errno != EAGAIN)
        throw std::runtime_error(std::string("Failed to send TCP message: ") + strerror(errno));

    /* TODO: Instead of busy waiting like this we should keep a queue for every client
     * and poll their FDs to see when they're ready */
    if (bytesSent <= 0 && (errno == EWOULDBLOCK || errno == EAGAIN)) {
        send(client, data, size);
        return;
    }

    if (bytesSent > 0 && bytesSent < size) {
        const auto remainingBufferPtr = data + bytesSent;
        const auto remainingSize = size - bytesSent;
        send(client, remainingBufferPtr, remainingSize);
    }
}

void TCPServer::send(const ClientHandle &client, const PacketBuffer &data, const ssize_t size) {
    send(client, data.get(), size);
}

void TCPServer::sendToAll(const PacketBuffer &data, const ssize_t size) const {
    for (const auto &client: clientManager->getAllClients() | std::views::values) {
        send(client, data, size);
    }
}

void TCPServer::sendToAllExcept(const PacketBuffer &data, const ssize_t size, const ClientHandle &except) const {
    for (const auto &client: clientManager->getAllClients() | std::views::values) {
        if (client.id != except.id)
            send(client, data, size);
    }
}
void TCPServer::sendToAllInLobby(const PacketBuffer &buf, ssize_t size) const {
    for (auto &pair : clientManager->getAllClients()) {
        const auto &client = pair.second;
        if (client.state == ClientStateLobby::InLobby) {
            send(client, buf.get(), size);
        }
    }
}

void TCPServer::sendToAllInLobbyExcept(const PacketBuffer &buf, ssize_t size, const ClientHandle &exclude) const {
    for (auto &pair : clientManager->getAllClients()) {
        const auto &client = pair.second;
        if (client.state == ClientStateLobby::InLobby && client.id != exclude.id) {
            send(client, buf.get(), size);
        }
    }
}
void TCPServer::sendToAllExcept(const PacketBuffer &data, const ssize_t size, const uint16_t exceptId) const {
    const auto client = clientManager->getClient(exceptId);
    sendToAllExcept(data, size, *client);
}

void TCPServer::receivePacketFromClient(ClientHandle &client) {
    char headerBuf[sizeof(TCPPacketHeader)];
    const ssize_t headerBytesRead = recv(client.tcpSocketFd, headerBuf, sizeof(headerBuf), MSG_WAITALL);

    if (headerBytesRead <= 0)
        throw std::runtime_error("Error while reading the header of client packet");

    auto header = TCPPacketHeader();
    std::memcpy(&header, headerBuf, sizeof(TCPPacketHeader));

    if (header.payloadSize > MAX_TCP_PAYLOAD_SIZE) {
        std::cerr << "Client sent a packet with payload too large! Size: " << header.payloadSize << std::endl;
        clientManager->removeClient(client.tcpSocketFd);
        return;
    }

    const auto payloadBuf = std::make_unique<char[]>(header.payloadSize);

    if (header.payloadSize > 0) {
        const ssize_t payloadBytesRead = recv(client.tcpSocketFd, payloadBuf.get(), header.payloadSize, MSG_WAITALL);
        if (payloadBytesRead <= 0)
            throw std::runtime_error("Error while reading the payload from client");
    }

    handlePacket(header.type, std::move(payloadBuf), header.payloadSize, client);
}

void TCPServer::handlePacket(TCPPacketType type, const PacketBuffer &payload, const ssize_t size,
                             ClientHandle &client) {
    try {
        switch (type) {
            case TCPPacketType::Name:
                NameHandler::handle(std::string(payload.get(), size), client, this);
                break;

            case TCPPacketType::UdpInfo:
                UdpInfoHandler::handle(std::move(payload), client, clientManager);
                break;

            default:
                std::cerr << "Received a packet with unknown id: " << static_cast<uint8_t>(type) << std::endl;
        }
    } catch (DeserializationError &e) {
        std::cerr << "Error while deserializing packet: " << e.what() << std::endl;
    }
}
void TCPServer::notifyClientDisconnected(const ClientHandle& client) const {
    const auto [packet,packetSize] = TCPPacket::create<ClientDisconnectedPacket>(
        client.nick.c_str(), client.nick.size());
    const auto buf = TCPPacket::serialize(packet);

    sendToAllInLobbyExcept(std::move(buf),
                           packetSize,
                           client);
}


[[noreturn]]
void TCPServer::loop() {
    int efd = epoll_create1(0);
    if (efd < 0)
        throw std::runtime_error(std::string("epoll_create1 failed: ") + strerror(errno));
    epoll_event ev{};
    ev.events = EPOLLIN;
    ev.data.fd = socketFd;
    epoll_ctl(efd, EPOLL_CTL_ADD, socketFd, &ev);
    std::cout << "waiting for clients...\n";
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
                    const int cfd = accept(socketFd, reinterpret_cast<sockaddr *>(&cli), &clilen);
                    if (cfd < 0) break;
                    makeNonBlocking(cfd);
                    auto *client = clientManager->newClient(cli, cfd);
                    client->state = ClientStateLobby::WaitingForNick;
                    auto packet = ProvideNamePacket();
                    const auto packetBuf = TCPPacket::serialize(packet);
                    send(*client, packetBuf, sizeof(packet));

                    epoll_event cev{};
                    cev.events = EPOLLIN | EPOLLRDHUP;
                    cev.data.fd = cfd;
                    epoll_ctl(efd, EPOLL_CTL_ADD, cfd, &cev);
                    std::cout << "\nAccepted client fd=" << cfd << "\n";
                }
                continue;
            }
            if (events[i].events & (EPOLLRDHUP | EPOLLHUP | EPOLLERR)) {
                auto client = clientManager->getClientByFd(fd);

                if (client && client->state == ClientStateLobby::InLobby) {
                    notifyClientDisconnected(*client);
                }
                clientManager->removeClient(fd);

                TimeUntilStartPacket countdown{};
                countdown.seconds = timeUntilStart();
                auto countdownBuf = TCPPacket::serialize(countdown);
                sendToAllInLobby(countdownBuf, sizeof(countdown));
                continue;
            }
            if (events[i].events & EPOLLIN) {
                const auto client = clientManager->getClientByFd(fd);
                if (!client) continue;

                receivePacketFromClient(*clientManager->getClientByFd(fd));
            }
        }
    }
}

void TCPServer::broadcastPlayers() const {
    const auto &clients = clientManager->getAllClients();
    size_t connectedCount = 0;
    for (const auto &[id, client]: clients) {
        if (client.connected && client.state == ClientStateLobby::InLobby)
            ++connectedCount;
    }
    std::string lobbyMessage = "Player Count (" + std::to_string(connectedCount) + "):\n";

    int index = 1;
    for (const auto &[id, client]: clients) {
        if (!client.connected || client.state != ClientStateLobby::InLobby) continue;
        lobbyMessage += std::to_string(index++) + ". " + client.nick + "\n";
    }
    lobbyMessage += "Race starts in: " + std::to_string(timeUntilStart()) + "s\n";
    for (const auto &[id, client]: clients) {
        if (!client.connected || client.state != ClientStateLobby::InLobby) continue;
        send(client, lobbyMessage.c_str(), lobbyMessage.size());
    }
}
