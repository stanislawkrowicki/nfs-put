#include "opponent_manager.hpp"

#include "default_vehicle_model.hpp"
#include "vehicle_manager.hpp"

OpponentManager &OpponentManager::getInstance() {
    static OpponentManager instance;
    return instance;
}

void OpponentManager::updateOpponent(const uint16_t clientId, const char *state) {
    const auto vehicle = vehicleMap.find(clientId);

    if (vehicle == vehicleMap.end()) {
        addNewOpponent(clientId);
        return;
    }

    btTransform transform;
    float velocityData[3];
    btScalar steeringAngle;

    btTransformFloatData transformData{};

    std::memcpy(&transformData, state, sizeof(transformData));
    std::memcpy(&velocityData, state + sizeof(transformData), sizeof(velocityData));
    std::memcpy(&steeringAngle, state + sizeof(transformData) + sizeof(velocityData), sizeof(btScalar));

    transform.deSerialize(transformData);
    const auto velocity = btVector3(velocityData[0], velocityData[1], velocityData[2]);

    const auto btVehicle = vehicle->second->getBtVehicle();

    auto *motionState = dynamic_cast<btDefaultMotionState *>(vehicle->second->
        getBtChassis()->getMotionState());

    if (motionState) {
        motionState->setWorldTransform(transform);
    }

    btVehicle->getRigidBody()->setWorldTransform(
        transform);
    btVehicle->getRigidBody()->setLinearVelocity(velocity);
    btVehicle->setSteeringValue(steeringAngle, 0);
    btVehicle->setSteeringValue(steeringAngle, 1);
}

void OpponentManager::addNewOpponent(uint16_t clientId) {
    const VehicleConfig config;

    config.isPlayerVehicle = false;
    config.bodyColor = glm::vec4(0.3f, 1.0f, 0.4f, 1.0f);
    auto veh = VehicleManager::getInstance().createVehicle(
        config, VehicleModelCache::getDefaultVehicleModel());

    vehicleMap.insert({clientId, veh});
}
