#pragma once

#include "default_vehicle_model.hpp"
#include "vehicle_manager.hpp"
#include "netcode/client/opponent_vehicle_map.hpp"
#include "netcode/shared/packets/udp/server/opponent_positions_packet.hpp"

class PositionResponseHandler {
public:
    static void handle(const PacketBuffer &buf, const ssize_t size, const uint16_t localClientId) {
        auto packet = deserializeOpponentPositionsPacket(buf, size);

        for (const auto [clientId, position]: packet.positions) {
            /* TODO: See Loop::sendPositions() todo */
            if (clientId == localClientId) continue;

            const auto opponentVehicle = opponentVehicleMap.find(clientId);

            if (opponentVehicle == opponentVehicleMap.end()) {
                const VehicleConfig config;

                config.isPlayerVehicle = false;
                config.bodyColor = glm::vec4(0.3f, 1.0f, 0.4f, 1.0f);
                auto veh = VehicleManager::getInstance().createVehicle(
                    config, VehicleModelCache::getDefaultVehicleModel());

                opponentVehicleMap.insert({clientId, veh});
            } else {
                btTransform transform;
                btTransformFloatData floatData{};

                std::memcpy(&floatData, position, sizeof(floatData));

                transform.deSerialize(floatData);

                auto *motionState = dynamic_cast<btDefaultMotionState *>(opponentVehicle->second->
                    getBtChassis()->getMotionState());

                if (motionState) {
                    motionState->setWorldTransform(transform);
                }

                opponentVehicle->second->getBtVehicle()->getRigidBody()->setWorldTransform(
                    transform);
            }
        }
    }
};

