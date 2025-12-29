#include "tcp_client.hpp"

#include <arpa/inet.h>
#include <fcntl.h>
#include <netdb.h>
#include <unistd.h>
#include <cstring>
#include <iostream>
#include <stdexcept>
#include <thread>
#include <atomic>
#include <regex>

#include "handlers/name_accepted_handler.hpp"
#include "handlers/name_taken_handler.hpp"
#include "handlers/provide_name_handler.hpp"
#include "netcode/server/client_handle.hpp"
#include "netcode/shared/packets/tcp/tcp_packet.hpp"
#include "netcode/shared/packets/tcp/tcp_packet_header.hpp"
#include "netcode/shared/packets/tcp/client/name_packet.hpp"

static int makeNonBlocking(const int fd) {
    int flags = fcntl(fd, F_GETFL, 0);
    return fcntl(fd, F_SETFL, flags | O_NONBLOCK);
}

TCPClient::TCPClient(std::shared_ptr<ClientState> state) {
    this->state = std::move(state);
};

TCPClient::~TCPClient() {
    if (socketFd >= 0) close(socketFd);
    if (epollFd >= 0) close(epollFd);
}

void TCPClient::connect(const char* host, const char* port) {
    addrinfo hints{}, *res;
    hints.ai_family   = AF_INET;
    hints.ai_socktype = SOCK_STREAM;

    int rv = getaddrinfo(host, port, &hints, &res);
    if (rv != 0)
        throw std::runtime_error(gai_strerror(rv));

    socketFd = ::socket(res->ai_family, res->ai_socktype, res->ai_protocol);
    if (socketFd < 0)
        throw std::runtime_error(strerror(errno));

    makeNonBlocking(socketFd);

    if (::connect(socketFd, res->ai_addr, res->ai_addrlen) < 0) {
        if (errno != EINPROGRESS)
            throw std::runtime_error(strerror(errno));
    }

    freeaddrinfo(res);

    epollFd = epoll_create1(0);
    if (epollFd < 0)
        throw std::runtime_error(strerror(errno));

    epoll_event ev{};
    ev.events = EPOLLIN | EPOLLRDHUP;
    ev.data.fd = socketFd;
    epoll_ctl(epollFd, EPOLL_CTL_ADD, socketFd, &ev);

    ev.events = EPOLLIN;
    ev.data.fd = STDIN_FILENO;
    epoll_ctl(epollFd, EPOLL_CTL_ADD, STDIN_FILENO, &ev);

    std::cout << "Connected to server\n";

    // Start countdown display thread
    localTimeLeft = 0;
    countdownThread = std::thread([this]() {
    while (true) {
        if (localTimeLeft > 0) {
            // Move cursor to the beginning of the line and overwrite
            std::cout << "\rRace starts in: " << localTimeLeft-- << "s" << std::flush;
        }
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
});
    countdownThread.detach();

    loop();
}

void TCPClient::send(const char *data, const size_t size) const {
    if (socketFd < 0) return;

    ssize_t sent = ::send(socketFd, data, size, 0);
    if (sent <= 0)
        throw std::runtime_error(strerror(errno));
}

void TCPClient::send(const PacketBuffer &buf, const size_t size) const {
    send(buf.get(), size);
}

[[noreturn]]
void TCPClient::loop() const {
    epoll_event events[2];

    while (true) {
        int n = epoll_wait(epollFd, events, 2, -1);
        if (n < 0)
            throw std::runtime_error(strerror(errno));

        for (int i = 0; i < n; ++i) {
            if (events[i].events & (EPOLLHUP | EPOLLRDHUP | EPOLLERR)) {
                std::cout << "Disconnected from server\n";
                exit(0);
            }

            if (events[i].data.fd == socketFd) {
                receivePacket();
            }
            else if (events[i].data.fd == STDIN_FILENO) {
                handleUserInput();
            }
        }
    }
}

void TCPClient::receivePacket() const {
    char headerBuf[sizeof(TCPPacketHeader)];
    const ssize_t headerBytesRead = recv(socketFd, headerBuf, sizeof(headerBuf), MSG_WAITALL);

    if (headerBytesRead <= 0)
        throw std::runtime_error("Server closed connection");

    auto header = TCPPacketHeader();
    std::memcpy(&header, headerBuf, sizeof(TCPPacketHeader));

    if (header.payloadSize > MAX_TCP_PAYLOAD_SIZE) {
        throw std::runtime_error("Packet too large!");
    }

    const auto payloadBuf = std::make_unique<char[]>(header.payloadSize);

    if (header.payloadSize > 0) {
        const ssize_t payloadBytesRead = recv(socketFd, payloadBuf.get(), header.payloadSize, MSG_WAITALL);

        if (payloadBytesRead <= 0)
            throw std::runtime_error("Server closed connection");
    }

    handlePacket(header.type, std::move(payloadBuf), header.payloadSize);
    return;

    headerBuf[headerBytesRead] = '\0';
    std::string msg(headerBuf);
    if (!state->ready && msg.find("RACE BEGINS! - said the TCP server") != std::string::npos) {
        {
            std::lock_guard<std::mutex> lock(state->mtx);
            state->ready = true;
        }
        state->cv.notify_all();
        return;
    }
    // Parse race time from server message
    std::regex re("Race starts in: (\\d+)s");
    std::smatch match;
    if (std::regex_search(msg, match, re)) {
        int newTime = std::stoi(match[1]);
        localTimeLeft = newTime;
        lastLobbyMessage.clear();
        auto pos = msg.find("Race starts in:");
        if (pos != std::string::npos) {
            lastLobbyMessage = msg.substr(0, pos);  // everything before countdown
        }

        if (!lastLobbyMessage.empty()) {
            // Print leaderboard/chat above countdown line
            std::cout << "\n" << lastLobbyMessage << std::flush;
        }
    } else {
        // Other messages, just print normally
        std::cout << "\n" << msg << std::flush;
    }
}


void TCPClient::handleUserInput() const {
    char buf[32];
    const ssize_t bytes = read(STDIN_FILENO, buf, sizeof(buf));

    auto packet = NamePacket();
    std::memcpy(&packet.payload, buf, 32);

    if (bytes > 0) {
        send(TCPPacket::serialize(packet), sizeof(packet.header) + bytes);
    }
}

void TCPClient::handlePacket(const TCPPacketType type, const PacketBuffer &payload, const ssize_t size) const {
    try {
        switch (type) {
            case TCPPacketType::ProvideName:
                ProvideNameHandler::handle();
                break;
            case TCPPacketType::NameAccepted:
                NameAcceptedHandler::handle();
                break;
            case TCPPacketType::NameTaken:
                NameTakenHandler::handle();
                break;
            case TCPPacketType::TimeUntilStart:
                std::cout << "Received time until start" << std::endl;
                break;
            case TCPPacketType::RaceStart: {
                std::lock_guard<std::mutex> lock(state->mtx);
                state->ready = true;
            }
                state->cv.notify_all();
                break;

            default:
                std::cerr << "Received packet with unknown type: " << static_cast<uint8_t>(type) << std::endl;
        }
    } catch (DeserializationError &e) {
        std::cerr << "Error while deserializing packet: " << e.what() << std::endl;
    }
}
