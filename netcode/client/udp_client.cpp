#include "udp_client.hpp"
#include <arpa/inet.h>
#include <unistd.h>
#include <format>
#include <iostream>
#include <netdb.h>

#include "../shared/packets/udp/client/state_packet.hpp"
#include "handlers/opponent_states_handler.hpp"
#include "netcode/shared/client_inputs.hpp"
#include "netcode/shared/packets/udp/client/ping_packet.hpp"

UDPClient::UDPClient() {
    addrinfo hints{};
    addrinfo *result{};

    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_DGRAM;
    hints.ai_protocol = IPPROTO_UDP;

    const int status = getaddrinfo(SERVER_IP, SERVER_PORT, &hints, &result);
    if (status != 0)
        throw std::runtime_error(std::format("getaddrinfo failed: {}", gai_strerror(status)));

    bool connectedSuccessfully = false;

    for (; result != nullptr; result = result->ai_next) {
        socketFd = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
        if (socketFd < 0)
            continue;

        if (connect(socketFd, result->ai_addr, result->ai_addrlen) == 0) {
            std::cout << "Connected to the server!" << std::endl;
            connectedSuccessfully = true;
            break;
        }
    }

    freeaddrinfo(result);

    if (!connectedSuccessfully) {
        close();
        throw std::runtime_error(std::format("Failed to connect to the server with IP {} and port {}.", SERVER_IP,
                                             SERVER_PORT));
    }
}

UDPClient::~UDPClient() {
    if (socketFd >= 0)
        ::close(socketFd);
}

void UDPClient::send(const char *data, const ssize_t size) const {
    write(socketFd, data, size);
}

void UDPClient::send(const PacketBuffer &data, const ssize_t size) const {
    write(socketFd, data.get(), size);
}

void UDPClient::sendVehicleState(const std::shared_ptr<Vehicle> &vehicle, ClientInputs inputs) {
    const auto btVehicle = vehicle->getBtVehicle();

    const auto transform = btVehicle->getChassisWorldTransform();
    const auto velocity = btVehicle->getRigidBody()->getLinearVelocity();
    const auto steeringAngle = btVehicle->getSteeringValue(0);

    btTransformFloatData transformData{};
    transform.serialize(transformData);

    float velocityData[3];
    velocityData[0] = velocity.getX();
    velocityData[1] = velocity.getY();
    velocityData[2] = velocity.getZ();

    constexpr auto transformSize = sizeof(btTransformFloatData);
    constexpr auto velocitySize = sizeof(velocityData);
    constexpr auto steeringAngleSize = sizeof(btScalar);

    StateBuffer buf;

    std::memcpy(buf, &transformData, transformSize);
    std::memcpy(buf + transformSize, &velocityData, velocitySize);
    std::memcpy(buf + transformSize + velocitySize, &steeringAngle, steeringAngleSize);
    std::memcpy(buf + sizeof(buf) - sizeof(inputs), &inputs, sizeof(inputs));

    const auto packet = UDPPacket::create<StatePacket>(lastPacketId, buf, RACE_START_PAYLOAD_SIZE);

    send(UDPPacket::serialize(packet), sizeof(packet));
    lastPacketId++;
}

void UDPClient::handlePacket(const PacketBuffer &buf, const ssize_t size) const {
    const bool isValid = UDPPacket::validate(buf, size);
    if (!isValid) {
        std::cerr << "Received a packet with invalid checksum." << std::endl;
        return;
    }

    UDPPacketType type;
    std::memcpy(&type, buf.get(), sizeof(UDPPacketType));

    try {
        switch (type) {
            case UDPPacketType::PositionResponse:
                OpponentStatesHandler::handle(buf, size);
                break;

            default:
                std::cerr << "Received packet with unknown type!" << std::endl;
        }
    } catch (DeserializationError &e) {
        std::cerr << "Error while deserializing packet: " << e.what() << std::endl;
    }
}

void UDPClient::listen() {
    waitForMessages = true;

    while (waitForMessages) {
        auto buf = std::make_unique<char []>(MAX_MESSAGE_SIZE);
        const ssize_t bytesRead = ::read(socketFd, buf.get(), MAX_MESSAGE_SIZE);

        if (bytesRead < 0) {
            std::cerr << "Error while reading data from UDP connection: " << strerror(errno) << std::endl;
            continue;
        }

        handlePacket(buf, bytesRead);
    }
}

void UDPClient::stopListening() {
    waitForMessages = false;
}

void UDPClient::close() {
    ::close(socketFd);
    socketFd = -1;
}

uint16_t UDPClient::getPort() const {
    sockaddr_in addr{};
    socklen_t size = sizeof(addr);

    getsockname(socketFd, reinterpret_cast<sockaddr *>(&addr), &size);

    return ntohs(addr.sin_port);
}


