#pragma once
#include <map>
#include <memory>

#include "vehicle.hpp"
#include "netcode/shared/client_inputs.hpp"
#include "netcode/shared/opponent_info.hpp"

class OpponentManager {
    std::vector<std::pair<uint16_t, VehicleConfig> > vehiclesToCreate;
    std::map<uint16_t, std::shared_ptr<Vehicle> > vehicleMap;
    std::map<uint16_t, ClientInputs> inputsMap;

    bool openglReady = false;

    OpponentManager() = default;

    void enqueueVehicleCreationForOpponent(uint16_t opponentId, const VehicleConfig &config);

    void createOpponentVehicle(uint16_t opponentId, const VehicleConfig &config);

public:
    static OpponentManager &getInstance();

    OpponentManager(const OpponentManager &) = delete;

    OpponentManager &operator=(const OpponentManager &) = delete;

    OpponentManager(OpponentManager &&) = delete;

    OpponentManager &operator=(OpponentManager &&) = delete;

    void updateOpponentState(uint16_t clientId, const char *state);

    void addNewOpponent(const uint16_t &opponentId, uint8_t gridPositionIndex, const PlayerVehicleColor &vehicleColor);

    void applyLastInputs(float dt);

    void setOpenGLReady();
};

