#pragma once

#include "bsd_server.hpp"
#include "../shared/packets/udp/udp_packet.hpp"

#define MAX_PACKET_SIZE 1024

class UDPServer final : public BSDServer {
public:
    explicit UDPServer(std::shared_ptr<ClientManager> clientManager);

    ~UDPServer() override;

    void listen(const char *port) override;

    void send(ClientHandle client, const PacketBuffer &data, ssize_t size) const override;

    void sendToAll(const PacketBuffer &data, ssize_t size) const override;

    void sendToAllExcept(const PacketBuffer &data, ssize_t size, const ClientHandle &except) const;

    void sendToAllExcept(const PacketBuffer &data, ssize_t size, uint16_t exceptId) const;

    void handlePacket(const PacketBuffer &buf, ssize_t size, ClientHandle &client) const;

    [[nodiscard]]
    std::unordered_map<uint16_t, ClientHandle> &getAllClients() const;

private:
    int socketFd;

    [[noreturn]] void loop() const;
};
