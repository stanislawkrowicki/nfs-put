#include "opponent_manager.hpp"

#include "default_vehicle_model.hpp"
#include "netcode/shared/starting_positions.hpp"
#include "vehicle_manager.hpp"
#include "netcode/shared/client_inputs.hpp"
#include "netcode/shared/opponent_info.hpp"
#include "netcode/shared/packets/udp/client/state_packet.hpp"

void OpponentManager::enqueueVehicleCreationForOpponent(uint16_t opponentId, const VehicleConfig &config) {
    if (!openglReady)
        vehiclesToCreate.emplace_back(opponentId, config);
    else
        createOpponentVehicle(opponentId, config);
}

void OpponentManager::createOpponentVehicle(uint16_t opponentId, const VehicleConfig &config) {
    if (vehicleMap.contains(opponentId)) {
        std::cerr << "Tried to create an opponent vehicle for a client with existing vehicle" << std::endl;
        return;
    }

    auto veh = VehicleManager::getInstance().createVehicle(
        config, VehicleModelCache::getDefaultVehicleModel());

    vehicleMap.insert({opponentId, veh});
}

OpponentManager &OpponentManager::getInstance() {
    static OpponentManager instance;
    return instance;
}

void OpponentManager::updateOpponentState(const uint16_t clientId, const char *state) {
    const auto vehicle = vehicleMap.find(clientId);

    if (vehicle == vehicleMap.end()) {
        return;
    }

    btTransform transform;
    float velocityData[3];
    btScalar steeringAngle;
    ClientInputs inputs;

    btTransformFloatData transformData{};

    std::memcpy(&transformData, state, sizeof(transformData));
    std::memcpy(&velocityData, state + sizeof(transformData), sizeof(velocityData));
    std::memcpy(&steeringAngle, state + sizeof(transformData) + sizeof(velocityData), sizeof(steeringAngle));
    std::memcpy(&inputs, state + STATE_PAYLOAD_SIZE - sizeof(inputs), sizeof(inputs));

    transform.deSerialize(transformData);
    const auto velocity = btVector3(velocityData[0], velocityData[1], velocityData[2]);

    const auto btVehicle = vehicle->second->getBtVehicle();

    auto *motionState = dynamic_cast<btDefaultMotionState *>(vehicle->second->
        getBtChassis()->getMotionState());

    if (motionState) {
        motionState->setWorldTransform(transform);
    }

    btVehicle->getRigidBody()->setWorldTransform(transform);
    btVehicle->getRigidBody()->setLinearVelocity(velocity);
    btVehicle->setSteeringValue(steeringAngle, 0);
    btVehicle->setSteeringValue(steeringAngle, 1);

    inputsMap[clientId] = inputs;
}

void OpponentManager::addNewOpponent(const uint16_t &opponentId, const uint8_t gridPositionIndex,
                                     const PlayerVehicleColor &vehicleColor) {
    const VehicleConfig config;

    const auto gridPosition = startingPositions[gridPositionIndex % std::size(startingPositions)];

    config.isPlayerVehicle = false;
    config.bodyColor = glm::vec4(vehicleColor.rNormalized(), vehicleColor.gNormalized(), vehicleColor.bNormalized(),
                                 1.0f);
    config.position = gridPosition.getOrigin();
    config.rotation = gridPosition.getRotation();

    enqueueVehicleCreationForOpponent(opponentId, config);
}

void OpponentManager::applyLastInputs(const float dt) {
    for (const auto &[clientId, vehicle]: vehicleMap) {
        const auto inputBitmapIterator = inputsMap.find(clientId);

        if (inputBitmapIterator == inputsMap.end())
            continue;

        const auto inputBitmap = inputBitmapIterator->second;

        const auto forward = (inputBitmap & INPUT_THROTTLE) > 0;
        const auto backward = (inputBitmap & INPUT_BRAKE) > 0;
        const auto handbrake = (inputBitmap & INPUT_HANDBRAKE) > 0;
        const auto left = (inputBitmap & INPUT_LEFT) > 0;
        const auto right = (inputBitmap & INPUT_RIGHT) > 0;

        vehicle->updateControls(forward, backward, handbrake, left, right, dt);
    }
}

void OpponentManager::setOpenGLReady() {
    openglReady = true;

    for (const auto &[opponentId, vehicleConfig]: vehiclesToCreate)
        createOpponentVehicle(opponentId, vehicleConfig);

    vehiclesToCreate.clear();
}
