#pragma once
#include "physics.hpp"
#include "vehicle_config.hpp"

class Vehicle {
    btDynamicsWorld *dynamicsWorld;

    VehicleConfig config;

    btCompoundShape *chassisShape{};
    btMotionState *chassisMotion{};
    btRigidBody *chassis{};

    btVehicleRaycaster *raycaster{};
    btRaycastVehicle *btVehicle{};

    std::shared_ptr<Model> model;

    void createBtVehicle();

public:
    explicit Vehicle(VehicleConfig config, std::shared_ptr<Model> vehicleModel);

    ~Vehicle();

    void addToWorld() const;

    [[nodiscard]]
    btRaycastVehicle *getBtVehicle() const;

    [[nodiscard]]
    std::string getName() const;

    [[nodiscard]]
    VehicleConfig getConfig() const;

    [[nodiscard]]
    std::shared_ptr<Model> getModel() const;

    void updateControls(bool forward, bool backward, bool handbrake, bool left, bool right, float dt) const;
};
