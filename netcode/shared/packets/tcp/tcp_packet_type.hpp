#pragma once

#include <cstdint>

enum class TCPPacketType : uint8_t {
    Name,
    ProvideName,
    NameTaken,
    NameAccepted,
    TimeUntilStart,
    UdpInfo,
    StartGame,
    ClientConnected,
    ClientDisconnected,
    LobbyClientList,
    OpponentsInfo
};
