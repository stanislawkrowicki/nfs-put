#pragma once

#include <string>
#include <vector>
#include <sys/epoll.h>
#include <thread>
#include <atomic>
#include <condition_variable>

#include "udp_client.hpp"
#include "netcode/shared/packets/tcp/tcp_packet.hpp"
#include "netcode/shared/packets/tcp/server/start_game_packet.hpp"

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

    std::string getPlayerNickname() const;

    void connect(const char *host, const char *port);

    [[noreturn]]
    void listen();

    void send(const char *data, size_t size) const;

    void send(const PacketBuffer &buf, size_t size) const;

    void displayLobby();

    void setId(uint16_t id);

    uint16_t getId() const;

    void setGameReady();

    void setRaceStartTime(std::chrono::time_point<std::chrono::steady_clock> time);

    int getTimeUntilRaceStart() const;

    bool isRaceStartCountdownActive() const;

    uint8_t getGridPosition() const;

    void setGridPosition(uint8_t gridPos);

    void setColor(PlayerVehicleColor vehicle_color);

    PlayerVehicleColor getColor() const;

    void sendLapCount(uint8_t lapCount) const;

    void setUdpBridge(const std::shared_ptr<UDPClient> &udpClient);

    void setRaceEndSeconds(uint8_t seconds);

    bool isRaceEndCountdownActive() const;

    int getRaceEndSeconds() const;
    void closeWindow() const;

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

    std::atomic<int> raceEndSeconds{ -1 };

    uint8_t gridPosition;
    PlayerVehicleColor vehicleColor;
    uint16_t clientId;

    std::chrono::time_point<std::chrono::steady_clock> raceStartTime;
    bool countdownUntilStart{false};

    void receivePacket();

    void handleUserInput() const;

    void handlePacket(TCPPacketType type, const PacketBuffer &payload, ssize_t size);

    std::shared_ptr<ClientState> state;

    std::shared_ptr<UDPClient> udpBridge{};
};
