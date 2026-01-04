#pragma once

#include "../../shared/packets/tcp/server/handshake_ack_packet.hpp"
#include "../../shared/packets/udp/client/handshake_packet.hpp"

class HandshakeHandler {
public:
    static void handle(const HandshakePacket &packet, const std::shared_ptr<ClientManager> &clientManager,
                       const sockaddr_in &sender, const std::shared_ptr<TCPServer> &tcpBridge) {
        uint16_t clientId;
        std::memcpy(&clientId, packet.payload, sizeof(clientId));

        const auto client = clientManager->getClient(clientId);

        if (!client) {
            std::cerr << "Handshake subject not found" << std::endl;
            return;
        }

        clientManager->updateClientUdpAddr(*client, sender);

        if (tcpBridge) {
            const auto [responsePacket, responsePacketSize] = TCPPacket::create<HandshakeAckPacket>(nullptr, 0);
            tcpBridge->send(*client, TCPPacket::serialize(responsePacket), responsePacketSize);
        } else {
            std::cerr << "Tried to make UDP handshake without tcpBridge set" << std::endl;
        }
    };
};
