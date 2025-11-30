#pragma once

#include <netinet/in.h>

struct ClientHandle {
    int socketFd;
    sockaddr_in address;
    uint16_t id;
    bool connected;
};
