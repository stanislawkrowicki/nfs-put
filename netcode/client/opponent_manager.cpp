#include "opponent_manager.hpp"

#include "default_vehicle_model.hpp"
#include "vehicle_manager.hpp"

OpponentManager &OpponentManager::getInstance() {
    static OpponentManager instance;
    return instance;
}

void OpponentManager::updateOpponent(const uint16_t clientId, const char *position) {
    const auto vehicle = vehicleMap.find(clientId);

    if (vehicle == vehicleMap.end()) {
        addNewOpponent(clientId);
        return;
    }

    btTransform transform;
    btTransformFloatData floatData{};

    std::memcpy(&floatData, position, sizeof(floatData));

    transform.deSerialize(floatData);

    auto *motionState = dynamic_cast<btDefaultMotionState *>(vehicle->second->
        getBtChassis()->getMotionState());

    if (motionState) {
        motionState->setWorldTransform(transform);
    }

    vehicle->second->getBtVehicle()->getRigidBody()->setWorldTransform(
        transform);
}

void OpponentManager::addNewOpponent(uint16_t clientId) {
    const VehicleConfig config;

    config.isPlayerVehicle = false;
    config.bodyColor = glm::vec4(0.3f, 1.0f, 0.4f, 1.0f);
    auto veh = VehicleManager::getInstance().createVehicle(
        config, VehicleModelCache::getDefaultVehicleModel());

    vehicleMap.insert({clientId, veh});
}
