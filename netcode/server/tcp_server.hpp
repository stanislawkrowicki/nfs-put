#pragma once

#include "bsd_server.hpp"
#include "client_manager.hpp"
#include <string>
#include <chrono>
#include <condition_variable>
#include "../shared/packets/tcp/tcp_packet.hpp"

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

    std::shared_ptr<ClientManager> clientManager;
    std::shared_ptr<ServerState> state;

    void listen(const char *port);

    //void addMessageListener(std::function<void(const Packet &)>) override;

    static void send(const ClientHandle &client, const char *data, ssize_t size);

    //void send(ClientHandle client, const std::unique_ptr<char[]> &data, ssize_t size) const;
    //void sendToAll(const PacketBuffer &data, ssize_t size) const override;

    static void send(const ClientHandle &client, const PacketBuffer &data, ssize_t size);

    void sendToAll(const PacketBuffer &data, ssize_t size) const;

    void sendToAllExcept(const PacketBuffer &data, ssize_t size, const ClientHandle &except) const;

    void sendToAllExcept(const PacketBuffer &data, ssize_t size, uint16_t exceptId) const;

    void sendToAllInLobby(const PacketBuffer &buf, ssize_t size) const;

    void sendToAllInLobbyExcept(const PacketBuffer &data, ssize_t size, const ClientHandle &except) const;

    void sendToAllInGame(const PacketBuffer &data, ssize_t size) const;

    void receivePacketFromClient(ClientHandle &client);

    void handlePacket(TCPPacketType type, const PacketBuffer &payload, ssize_t size, ClientHandle &client);

    void notifyClientDisconnected(const ClientHandle &client) const;

    void sendClientOpponentsInfo(const ClientHandle &client) const;

    [[nodiscard]]
    int timeUntilStart() const;

    void countdownToLobbyEnd() const;

    void startRaceStartCountdown() const;
 void broadcastLapsUpdate(const ClientHandle &updatedClient) const;

private:
    int socketFd;

    const int lobbyEndTimeout{15};
    std::chrono::steady_clock::time_point lobbyStartTime;

    const int raceStartTimeout{5};

    [[noreturn]] void loop();

    void broadcastPlayers() const;
};
