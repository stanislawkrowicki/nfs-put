#include "laps.hpp"

#include <algorithm>
#include <iostream>

#include "lap_checkpoints.hpp"
#include "BulletCollision/CollisionDispatch/btGhostObject.h"
#include "BulletCollision/CollisionShapes/btBoxShape.h"
#include "BulletDynamics/Dynamics/btRigidBody.h"

uint16_t Laps::LOCAL_PLAYER_ID = UINT16_MAX;

void Laps::createCheckpoints() {
    for (const auto &checkpoint: LAP_CHECKPOINTS) {
        createCheckpoint(checkpoint);
    }
}

void Laps::createCheckpoint(const LapCheckpointPosition &checkpoint) {
    auto *ghost = new btGhostObject();
    ghost->setCollisionShape(new btBoxShape(checkpoint.dimensions));

    btTransform transform;
    transform.setOrigin(checkpoint.pos);
    transform.setRotation(checkpoint.rot);

    ghost->setWorldTransform(transform);

    ghost->setCollisionFlags(ghost->getCollisionFlags() | btCollisionObject::CF_NO_CONTACT_RESPONSE);

    dynamicsWorld->addCollisionObject(ghost, btBroadphaseProxy::SensorTrigger, btBroadphaseProxy::DefaultFilter);

    checkpoints.push_back(ghost);
}

void Laps::setLocalPlayerName(const std::string &playerName) {
    localPlayerName = playerName;
}

void Laps::initializeTracker(btDynamicsWorld *world) {
    this->dynamicsWorld = world;

    if (checkpoints.empty())
        createCheckpoints();
}

void Laps::addOpponent(const uint16_t playerId, const std::string &playerName) { {
        std::lock_guard lock(opponentsLapsMutex);
        if (opponentsLaps.contains(playerId)) {
            std::cerr << "Tried to add a player to the laps system that was already added." << std::endl;
            return;
        }

        opponentsLaps[playerId] = 0;
        opponentsNames[playerId] = playerName;
    }

    updateLeaderboard();
}

void Laps::addLocalPlayer(const std::string &playerName, btRigidBody *rigidBody) {
    localPlayerBody = rigidBody;
    localPlayerProgress = PlayerProgress{0, 0};
    localPlayerName = playerName;

    // This is to force the playerName to be added  into the leaderboard
    // TODO: We need to do it some other way. It's late, the deadline is tomorrow,
    // pls fix
    updateLeaderboard();
}

void Laps::setLapIncreaseCallback(const std::function<void(int)> &fun) {
    onLocalPlayerLapIncrease = fun;
}

void Laps::updateLocalPlayer() {
    const auto targetCheckpoint = checkpoints[localPlayerProgress.nextCheckpointIndex];

    bool isOverlapping = false;
    const int numOverlapping = targetCheckpoint->getNumOverlappingObjects();

    for (int i = 0; i < numOverlapping; i++) {
        if (targetCheckpoint->getOverlappingObject(i) == localPlayerBody) {
            isOverlapping = true;
            break;
        }
    }

    if (isOverlapping) {
        if (localPlayerProgress.nextCheckpointIndex == checkpoints.size() - 1) {
            localPlayerProgress.lapCount++;
            updateLeaderboard();
            onLocalPlayerLapIncrease(localPlayerProgress.lapCount);
        }

        localPlayerProgress.nextCheckpointIndex =
                (localPlayerProgress.nextCheckpointIndex + 1) % static_cast<int>(checkpoints.size());
    }
}

void Laps::setOpponentLaps(const uint16_t opponentId, const uint8_t lapsCount) { {
        std::lock_guard lock(opponentsLapsMutex);
        opponentsLaps[opponentId] = lapsCount;
    }
    updateLeaderboard();
}

uint8_t Laps::getLocalPlayerLaps() const {
    return localPlayerProgress.lapCount;
}

std::map<uint16_t, uint8_t> Laps::getOpponentLaps() const {
    return opponentsLaps;
}

std::vector<LeaderboardEntry> Laps::getLeaderboard() const {
    return leaderboard;
}

void Laps::updateLeaderboard() {
    std::vector<std::pair<uint16_t, uint8_t> > allPlayers(opponentsLaps.begin(), opponentsLaps.end());
    allPlayers.emplace_back(LOCAL_PLAYER_ID, localPlayerProgress.lapCount);

    std::ranges::sort(allPlayers, [](const auto &a, const auto &b) {
        if (a.second != b.second) return a.second > b.second;
        return a.first > b.first;
    });

    std::vector<LeaderboardEntry> result;
    bool localPlayerFoundInTop = false;

    const uint8_t numToRank = std::min(allPlayers.size(), static_cast<size_t>(LEADERBOARD_SIZE - 1));
    for (uint8_t i = 0; i < numToRank; ++i) {
        auto name = opponentsNames[allPlayers[i].first];

        if (allPlayers[i].first == LOCAL_PLAYER_ID) {
            localPlayerFoundInTop = true;
            name = localPlayerName;
        }

        result.emplace_back(LeaderboardEntry{
            allPlayers[i].first,
            name,
            allPlayers[i].second,
            static_cast<uint8_t>(i + 1)
        });
    }

    if (!localPlayerFoundInTop) {
        for (size_t i = numToRank; i < allPlayers.size(); ++i) {
            if (allPlayers[i].first == LOCAL_PLAYER_ID) {
                result.emplace_back(LeaderboardEntry{
                    allPlayers[i].first,
                    localPlayerName,
                    allPlayers[i].second,
                    static_cast<uint8_t>(i + 1)
                });
                break;
            }
        }
    } else {
        if (allPlayers.size() >= LEADERBOARD_SIZE)
            result.emplace_back(LeaderboardEntry{
                allPlayers[numToRank].first,
                opponentsNames[allPlayers[numToRank].first],
                allPlayers[numToRank].second,
                static_cast<uint8_t>(numToRank + 1)
            });
    }

    std::lock_guard lock(opponentsLapsMutex);
    leaderboard = result;
}

Laps::~Laps() {
    for (auto *cp: checkpoints) {
        if (dynamicsWorld) dynamicsWorld->removeCollisionObject(cp);
        delete cp->getCollisionShape();
        delete cp;
    }

    checkpoints.clear();
}
