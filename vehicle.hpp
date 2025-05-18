#pragma once
#include "physics.hpp"
#include "vehicle_config.hpp"

class Vehicle {
    btDynamicsWorld *dynamicsWorld;

    VehicleConfig config;

    btBoxShape *chassisShape{};
    btMotionState *chassisMotion{};
    btRigidBody *chassis{};

    btVehicleRaycaster *raycaster{};
    btRaycastVehicle *btVehicle{};

    void createBtVehicle();

public:
    explicit Vehicle(VehicleConfig config);

    ~Vehicle();

    void addToWorld() const;

    [[nodiscard]]
    btRaycastVehicle *getBtVehicle() const;

    [[nodiscard]]
    std::string getName() const;

    [[nodiscard]]
    VehicleConfig getConfig() const;

    void updateControls(bool forward, bool backward, bool handbrake, bool left, bool right, float dt) const;
};
