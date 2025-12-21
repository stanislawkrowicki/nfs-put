#pragma once

#include "bsd_server.hpp"
#include <string>
#include <chrono>

#define MAX_PACKET_SIZE 1024

class TCPServer final : public BSDServer {
public:
    explicit TCPServer(std::shared_ptr<ClientManager> clientManager);

    ~TCPServer() override;

    void listen(const char *port) override;

    void addMessageListener(std::function<void(const Packet &)>) override;

    void send(ClientHandle client, const char *data, ssize_t size) const override;
    int timeLeft() const;
    void startCountdownDisplay() const;
private:
    int socketFd;

    const int raceDuration{90};
    std::chrono::steady_clock::time_point raceStartTime;
    [[noreturn]] void loop() const;

    void parsePacket(const Packet &packet) const;
    void handleNick(const Packet &packet) const;
    void broadcastPlayers() const;
};
