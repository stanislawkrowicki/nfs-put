#pragma once

#include "bsd_server.hpp"

#define MAX_PACKET_SIZE 1024

class UDPServer final : public BSDServer {
public:
    explicit UDPServer(std::shared_ptr<ClientManager> clientManager);

    ~UDPServer() override;

    void listen(const char *port) override;

    void send(ClientHandle client, const char *data, ssize_t size) const override;

    void sendToAll(const char *data, ssize_t size) const override;

private:
    int socketFd;

    [[noreturn]] void loop() const;

    void parseBuf(std::unique_ptr<char[]> &buf, ssize_t size) const;
};
