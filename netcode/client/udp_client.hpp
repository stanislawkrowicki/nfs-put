#pragma once

#include "vehicle.hpp"
#include "../shared/packets/udp/udp_packet.hpp"
#include "LinearMath/btTransform.h"
#include "netcode/shared/client_inputs.hpp"

class UDPClient {
    static constexpr int MAX_MESSAGE_SIZE = 512;

    int socketFd = -1;
    long lastPacketId = 0;
    volatile bool waitForMessages = false;

public:
    UDPClient(const char *host, const char *port);

    ~UDPClient();

    void send(const char *data, ssize_t size) const;

    void send(const PacketBuffer &data, ssize_t size) const;

    void sendVehicleState(const std::shared_ptr<Vehicle> &vehicle, ClientInputs inputs);

    void handlePacket(const PacketBuffer &buf, ssize_t size) const;

    void listen();

    void stopListening();

    void close();

    [[nodiscard]]
    uint16_t getPort() const;
};
