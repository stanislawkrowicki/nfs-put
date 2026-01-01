#pragma once

#include <netinet/in.h>
#include <string>
enum class ClientStateLobby {
    WaitingForNick,
    InLobby
};
struct ClientHandle {
    int tcpSocketFd;
    mutable sockaddr_in udpAddr;
    uint16_t id;

    bool connected;
    int lastReceivedPacketId;
    std::string nick;
    ClientStateLobby state = ClientStateLobby::WaitingForNick;

    uint8_t gridPosition;
};
