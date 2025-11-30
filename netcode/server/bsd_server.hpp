#pragma once
#include <functional>
#include <memory>
#include <string>

#include "client_handle.hpp"
#include "client_manager.hpp"

struct Packet {
    std::unique_ptr<char[]> data;
    ssize_t size;
    ClientHandle *sender;
};

using PacketListener = std::function<void(const Packet &)>;

class BSDServer {
public:
    virtual ~BSDServer() = default;

    virtual void listen(const char *port) = 0;

    virtual void addMessageListener(PacketListener listener) = 0;

    virtual ssize_t send(ClientHandle client, const std::string &data) = 0;

protected:
    std::vector<PacketListener> listeners;
    uint16_t lastClientId = 0;
    std::shared_ptr<ClientManager> clientManager;
};
