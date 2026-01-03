#pragma once
#include <cstdint>
#include <functional>
#include <map>
#include <vector>

#include "lap_checkpoints.hpp"
#include "BulletCollision/CollisionDispatch/btGhostObject.h"
#include "BulletDynamics/Dynamics/btDiscreteDynamicsWorld.h"

struct PlayerProgress {
    uint8_t nextCheckpointIndex = 0;
    uint8_t lapCount = 0;
};

struct LeaderboardEntry {
    uint16_t playerId;
    uint8_t lapCount;
    uint8_t position;
};

class Laps {
    uint16_t LOCAL_PLAYER_ID = UINT16_MAX;
    uint8_t LEADERBOARD_SIZE = 4;

    btDynamicsWorld *dynamicsWorld = nullptr;
    std::map<uint16_t, uint8_t> opponentsLaps{};

    std::vector<LeaderboardEntry> leaderboard{};

    PlayerProgress localPlayerProgress{};
    btRigidBody *localPlayerBody{};

    std::vector<btGhostObject *> checkpoints{};

    std::function<void(int)> onLocalPlayerLapIncrease{};

    Laps() = default;

    ~Laps();

    void createCheckpoints();

    void createCheckpoint(const LapCheckpointPosition &checkpoint);

    void updateLeaderboard();

public:
    static Laps &getInstance() {
        static Laps instance;
        return instance;
    }

    Laps(const Laps &) = delete;

    void operator=(const Laps &) = delete;

    void initializeTracker(btDynamicsWorld *world);

    void addOpponent(uint16_t playerId);

    void addLocalPlayer(btRigidBody *rigidBody);

    void setLapIncreaseCallback(const std::function<void(int)> &fun);

    void updateLocalPlayer();

    void setOpponentLaps(uint16_t opponentId, uint8_t lapsCount);

    [[nodiscard]]
    uint8_t getLocalPlayerLaps() const;

    [[nodiscard]]
    std::map<uint16_t, uint8_t> getOpponentLaps() const;

    [[nodiscard]]
    std::vector<LeaderboardEntry> getLeaderboard() const;
};


