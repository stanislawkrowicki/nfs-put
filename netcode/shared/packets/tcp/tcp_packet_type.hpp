#pragma once

#include <cstdint>

enum class TCPPacketType : uint8_t {
    Name,
    ProvideName,
    NameTaken,
    NameAccepted,
    TimeUntilStart,
    UdpInfo,
    RaceStart,
    ClientConnected,
    ClientDisconnected,
    LobbyClientList,
    OpponentsInfo
};
