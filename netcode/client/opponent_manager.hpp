#pragma once
#include <chrono>
#include <map>
#include <memory>

#include "vehicle.hpp"
#include "netcode/shared/client_inputs.hpp"
#include "netcode/shared/opponent_info.hpp"

/* Time after which an opponent is marked as inactive */
constexpr int MAX_TIME_BETWEEN_PACKETS_MS = 1500;

class OpponentManager {
    std::vector<std::pair<uint16_t, VehicleConfig> > vehiclesToCreate;
    std::map<uint16_t, std::shared_ptr<Vehicle> > vehicleMap;
    std::map<uint16_t, ClientInputs> inputsMap;

    std::vector<uint16_t> inactiveOpponents{};
    std::map<uint16_t, std::chrono::time_point<std::chrono::steady_clock> > opponentsLastPacketTimestampMap{};

    uint32_t lastReceivedPacketId{0};

    bool openglReady = false;

    OpponentManager() = default;

    void enqueueVehicleCreationForOpponent(uint16_t opponentId, const VehicleConfig &config);

    void createOpponentVehicle(uint16_t opponentId, const VehicleConfig &config);

    bool isOpponentInactive(uint16_t clientId);

    void setOpponentInactive(uint16_t clientId);

    void setOpponentActive(uint16_t clientId);

public:
    static OpponentManager &getInstance();

    OpponentManager(const OpponentManager &) = delete;

    OpponentManager &operator=(const OpponentManager &) = delete;

    OpponentManager(OpponentManager &&) = delete;

    OpponentManager &operator=(OpponentManager &&) = delete;

    [[nodiscard]] bool isPacketLatest(uint32_t packetId) const;

    void setLatestPacket(uint32_t packetId);

    void updateInactiveOpponents();

    void updateOpponentState(uint16_t clientId, const char *state);

    void addNewOpponent(const uint16_t &opponentId, uint8_t gridPositionIndex, const PlayerVehicleColor &vehicleColor,
                        const std::string &nickname);

    void applyLastInputs(float dt);

    void setOpenGLReady();
};

