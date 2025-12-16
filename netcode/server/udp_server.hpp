#pragma once

#include "bsd_server.hpp"
#include "../shared/packets/udp/udp_packet.hpp"

#define MAX_PACKET_SIZE 1024

class UDPServer final : public BSDServer {
public:
    explicit UDPServer(std::shared_ptr<ClientManager> clientManager);

    ~UDPServer() override;

    void listen(const char *port) override;

    /* TODO: Use PacketBuffer instead of char * */
    void send(ClientHandle client, const char *data, ssize_t size) const override;

    void sendToAll(const char *data, ssize_t size) const override;

    void sendToAllExcept(const char *data, ssize_t size, const ClientHandle &except) const;

    void handlePacket(const PacketBuffer &buf, ssize_t size, const ClientHandle &client) const;

private:
    int socketFd;

    [[noreturn]] void loop() const;
};
