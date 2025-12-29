#include "opponent_manager.hpp"

#include "default_vehicle_model.hpp"
#include "vehicle_manager.hpp"
#include "netcode/shared/client_inputs.hpp"
#include "netcode/shared/packets/udp/client/state_packet.hpp"

OpponentManager &OpponentManager::getInstance() {
    static OpponentManager instance;
    return instance;
}

void OpponentManager::updateOpponentState(const uint16_t clientId, const char *state) {
    const auto vehicle = vehicleMap.find(clientId);

    if (vehicle == vehicleMap.end()) {
        addNewOpponent(clientId);
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
    std::memcpy(&inputs, state + RACE_START_PAYLOAD_SIZE - sizeof(inputs), sizeof(inputs));

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

void OpponentManager::addNewOpponent(uint16_t clientId) {
    const VehicleConfig config;

    config.isPlayerVehicle = false;
    config.bodyColor = glm::vec4(0.3f, 1.0f, 0.4f, 1.0f);
    auto veh = VehicleManager::getInstance().createVehicle(
        config, VehicleModelCache::getDefaultVehicleModel());

    vehicleMap.insert({clientId, veh});
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
