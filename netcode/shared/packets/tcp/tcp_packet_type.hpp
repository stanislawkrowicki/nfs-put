#pragma once

#include <cstdint>

enum class TCPPacketType : uint8_t {
    Name,
    ProvideName,
    NameTaken,
    NameAccepted,
    TimeUntilStart,
    StartGame,
    HandshakeAck,
    ClientConnected,
    ClientDisconnected,
    LobbyClientList,
    OpponentsInfo,
    ClientGameLoaded,
    RaceStartCountdown,
    LapCount,
    LapsUpdate,
    NameAcceptedButInQueue,
    QueueToLobby,
    RaceEndCountDownPacket
};
