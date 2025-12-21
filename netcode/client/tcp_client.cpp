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

static int makeNonBlocking(const int fd) {
    int flags = fcntl(fd, F_GETFL, 0);
    return fcntl(fd, F_SETFL, flags | O_NONBLOCK);
}

TCPClient::TCPClient() = default;

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
        if (localTimeLeft > 0 && !lastLobbyMessage.empty()) {
            // move cursor to top-left, clear screen
            std::cout << "\033[H\033[J";

            // replace the old "Race starts in: XXs" with updated timer
            std::string updated = std::regex_replace(
                lastLobbyMessage,
                std::regex("Race starts in: \\d+s"),
                "Race starts in: " + std::to_string(localTimeLeft--)
            );

            std::cout << updated << std::flush;
        }
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
});
    countdownThread.detach();

    loop();
}

void TCPClient::send(const char* data, ssize_t size) const {
    if (socketFd < 0) return;

    ssize_t sent = ::send(socketFd, data, size, 0);
    if (sent <= 0)
        throw std::runtime_error(strerror(errno));
}

[[noreturn]]
void TCPClient::loop() const {
    epoll_event events[4];

    while (true) {
        int n = epoll_wait(epollFd, events, 4, -1);
        if (n < 0)
            throw std::runtime_error(strerror(errno));

        for (int i = 0; i < n; ++i) {
            if (events[i].events & (EPOLLHUP | EPOLLRDHUP | EPOLLERR)) {
                std::cout << "Disconnected from server\n";
                exit(0);
            }

            if (events[i].data.fd == socketFd) {
                handleServerMessage();
            }
            else if (events[i].data.fd == STDIN_FILENO) {
                handleUserInput();
            }
        }
    }
}

void TCPClient::handleServerMessage() const {
    char buf[1024];
    ssize_t bytes = recv(socketFd, buf, sizeof(buf) - 1, 0);
    if (bytes <= 0)
        throw std::runtime_error("Server closed connection");

    buf[bytes] = '\0';
    std::string msg(buf);

    // Parse race time from server message
    std::regex re("Race starts in: (\\d+)s");
    std::smatch match;
    if (std::regex_search(msg, match, re)) {
        int newTime = std::stoi(match[1]);
        localTimeLeft = newTime;

        // Store the full message to redraw every second
        lastLobbyMessage = msg;
    } else {
        // Other messages, just print normally
        std::cout << "\n" << msg << std::flush;
    }
}


void TCPClient::handleUserInput() const {
    char buf[64];
    ssize_t bytes = read(STDIN_FILENO, buf, sizeof(buf));
    if (bytes > 0) {
        send(buf, bytes);
    }
}
