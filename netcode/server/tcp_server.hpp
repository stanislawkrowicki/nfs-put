#pragma once

#include "bsd_server.hpp"
#include "client_manager.hpp"
#include <string>
#include <chrono>
#include <condition_variable>

#define MAX_PACKET_SIZE 1024
struct ServerState {
    std::mutex mtx;
    std::condition_variable cv;
    bool udpReady = false;
};
class TCPServer final{
public:
    explicit TCPServer(std::shared_ptr<ClientManager> clientManager, std::shared_ptr<ServerState> state);

    ~TCPServer();

    void listen(const char *port);

    //void addMessageListener(std::function<void(const Packet &)>) override;

    void send(ClientHandle client, const char *data, ssize_t size) const;
    //void send(ClientHandle client, const std::unique_ptr<char[]> &data, ssize_t size) const;
    //void sendToAll(const PacketBuffer &data, ssize_t size) const override;

    int timeLeft() const;
    void startCountdown() const;
private:
    int socketFd;

    const int raceDuration{45};
    std::chrono::steady_clock::time_point raceStartTime;
    [[noreturn]] void loop() const;

    void parsePacket(const Packet &packet) const;
    void handleNick(const Packet &packet) const;
    void broadcastPlayers() const;
    std::shared_ptr<ClientManager> clientManager;
    std::shared_ptr<ServerState> state;
};
