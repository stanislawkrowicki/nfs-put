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

    float wheelRollingRotation[4] = {0, 0, 0, 0};

    float lastSteering = 0.0f;

    bool isBraking = false;

    void createBtVehicle();

    float calculateSteeringIncrement(float speed) const;

    [[nodiscard]]
    /* Bigger boost at low speeds; no boost at higher speed */
    float getEngineBoost() const;

public:
    explicit Vehicle(VehicleConfig config, std::shared_ptr<Model> vehicleModel);

    ~Vehicle();

    void addToWorld() const;

    [[nodiscard]]
    btRaycastVehicle *getBtVehicle() const;

    [[nodiscard]]
    btRigidBody *getBtChassis() const;

    [[nodiscard]]
    std::string getName() const;

    [[nodiscard]]
    VehicleConfig getConfig() const;

    [[nodiscard]]
    std::shared_ptr<Model> getModel() const;

    [[nodiscard]]
    glm::mat4 getOpenGLModelMatrix() const;

    [[nodiscard]]
    glm::vec3 getPosition() const;

    [[nodiscard]]
    bool getIsBraking() const;

    void updateControls(bool forward, bool backward, bool handbrake, bool left, bool right, float dt);

    /**
     * Analog version of updateControls for precise AI steering
     * @param forward bool
     * @param backward bool
     * @param steering float, wheels turning angle in radians
     */
    void aiUpdateControls(bool forward, bool backward, float steering);

    float applyRotationToWheel(size_t wheelIndex, float deltaRotation);

    void printDebugPosition() const;

    void freeze() const;

    void unfreeze() const;
};
