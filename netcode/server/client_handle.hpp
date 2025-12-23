#pragma once

#include <netinet/in.h>
#include <string>
enum class ClientStateLobby {
    WaitingForNick,
    InLobby
};
struct ClientHandle {
    int socketFd;
    sockaddr_in address;
    uint16_t id;
    bool connected;
    int lastReceivedPacketId;
    std::string nick;
    ClientStateLobby state = ClientStateLobby::WaitingForNick;
};
