#pragma once

#include <netinet/in.h>

enum class ClientState {
    WaitingForNick,
    InLobby
};

struct ClientHandle {
    int socketFd;
    sockaddr_in address;
    uint16_t id;
    bool connected;
    std::string nick;
    ClientState state = ClientState::WaitingForNick;
};
