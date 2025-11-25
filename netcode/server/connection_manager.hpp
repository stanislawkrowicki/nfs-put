#pragma once
#include <map>
#include "client_handle.hpp"

class ConnectionManager {
public:
    static ConnectionManager &getInstance();

    void setServerFd(int fd);

    void listen() const;

    void accept();

    ConnectionManager() = default;

    ~ConnectionManager() = default;

    ConnectionManager(const ConnectionManager &) = delete;

    ConnectionManager &operator=(const ConnectionManager &) = delete;

private:
    std::map<uint16_t, ClientHandle> clients;
    uint16_t lastClientId = 0;
    int serverFd = -1;
};
