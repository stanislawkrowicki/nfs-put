#pragma once

#include <string>
#include <vector>
#include <sys/epoll.h>
#include <thread>
#include <atomic>
#include <condition_variable>

#include "netcode/shared/packets/tcp/tcp_packet.hpp"

struct ClientState {
    std::mutex mtx;
    std::condition_variable cv;
    bool ready = false;
};
class TCPClient {
public:
    explicit TCPClient(std::shared_ptr<ClientState> state);
    ~TCPClient();

    void connect(const char* host, const char* port);

    void send(const char *data, size_t size) const;

    void send(const PacketBuffer &buf, size_t size) const;

private:
    int socketFd{-1};
    int epollFd{-1};

    mutable std::atomic<int> localTimeLeft{0};   // local countdown in seconds
    mutable std::thread countdownThread;         // background countdown thread
    mutable std::string lastLobbyMessage; // latest lobby + countdown from server

    [[noreturn]]
    void loop() const;

    void receivePacket() const;

    void handleUserInput() const;

    void handlePacket(TCPPacketType type, const PacketBuffer &payload, ssize_t size) const;

    std::shared_ptr<ClientState> state;
};
