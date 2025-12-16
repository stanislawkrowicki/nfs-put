#pragma once
#include <map>
#include <memory>

#include "vehicle.hpp"
#include "netcode/shared/client_inputs.hpp"

class OpponentManager {
    std::map<uint16_t, std::shared_ptr<Vehicle> > vehicleMap;
    std::map<uint16_t, ClientInputs> inputsMap;

    OpponentManager() = default;

public:
    static OpponentManager &getInstance();

    OpponentManager(const OpponentManager &) = delete;

    OpponentManager &operator=(const OpponentManager &) = delete;

    OpponentManager(OpponentManager &&) = delete;

    OpponentManager &operator=(OpponentManager &&) = delete;

    void updateOpponentState(uint16_t clientId, const char *state);

    void addNewOpponent(uint16_t clientId);

    void applyLastInputs(float dt);
};

