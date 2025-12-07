#pragma once

#include "bsd_server.hpp"
#include <string>

#define MAX_PACKET_SIZE 1024

class UDPServer final : public BSDServer {
public:
    explicit UDPServer(std::shared_ptr<ClientManager> clientManager);

    ~UDPServer() override;

    void listen(const char *port) override;

    void addMessageListener(std::function<void(const Packet &)>) override;

    void send(ClientHandle client, const char *data, ssize_t size) const override;

    void sendToAll(const char *data, ssize_t size) const override;

private:
    int socketFd;

    [[noreturn]] void loop() const;

    void parsePacket(const Packet &packet) const;
};
