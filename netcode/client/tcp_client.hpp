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
    void refreshScreen() const;
    void displayLobby() const;

    void connect(const char* host, const char* port);

    void send(const char *data, size_t size) const;

    void send(const PacketBuffer &buf, size_t size) const;

    void setGameReady();

    void setRaceStartTime(std::chrono::time_point<std::chrono::steady_clock> time);

    int getTimeUntilRaceStart() const;

    bool isRaceStartCountdownActive() const;

    uint8_t getGridPosition() const;

    void setGridPosition(uint8_t gridPos);

    void sendLapCount(uint8_t lapCount) const;

    mutable std::vector<std::string> lobbyNicks;
    mutable std::mutex lobbyMtx;
    mutable std::string localNick;
    mutable std::atomic<int> localTimeLeft{0};
    mutable std::atomic<bool> inLobby{false};


private:
    int socketFd{-1};
    int epollFd{-1};

    mutable std::thread countdownThread;         // background countdown thread
    mutable std::string lastLobbyMessage; // latest lobby + countdown from server

    uint8_t gridPosition;

    std::chrono::time_point<std::chrono::steady_clock> raceStartTime;
    bool countdownUntilStart{false};

    [[noreturn]]
    void loop();

    void receivePacket();

    void handleUserInput() const;

    void handlePacket(TCPPacketType type, const PacketBuffer &payload, ssize_t size);

    std::shared_ptr<ClientState> state;
};
