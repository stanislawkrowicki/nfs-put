#pragma once

#include <btBulletDynamicsCommon.h>
#include <BulletDynamics/Vehicle/btRaycastVehicle.h>
#include <vector>

struct WheelPlacement {
    btVector3 connectionPoint;
    bool isFrontWheel{};
};

struct VehicleConfig {
    /* General */
    mutable std::string name = "Vehicle";
    mutable btVector3 position = {0, 2, 0};
    mutable btQuaternion rotation = btQuaternion::getIdentity();

    /* Chassis */
    mutable btVector3 chassisHalfExtents = {0.87, 0.55, 2.4};
    mutable btScalar chassisMargin = 0.04f;

    /* The higher the offset, the lower the center of mass
     * Good values are something between 0.3 and 1.0 */
    mutable btScalar centerOfMassOffset = 0.8f;
    mutable btScalar mass = 1200.0f;

    /* Forces */
    mutable float engineForce = 5000.0f;
    mutable float brakingForce = 20000.0f; // used as negative engineForce to simplify, we should change that
    mutable float handbrakeForce = 3000.0f;

    /* Suspension */
    mutable btScalar suspensionStiffness = 40;
    mutable btScalar suspensionRestLength = 0.3f;
    mutable btScalar dampingCompression = 1.6f;
    mutable btScalar dampingRelaxation = 2.0f;

    /* Tuning */
    mutable btRaycastVehicle::btVehicleTuning tuning;

    /* Wheels */
    mutable btScalar wheelRadius = 0.4f;
    mutable btScalar wheelWidth = 0.6f; // only for graphics
    mutable btScalar frictionSlip = 2.0f;
    mutable btScalar rollInfluence = 0.2f;

    mutable float maxSteeringAngle = 0.5f; // radians
    mutable float minSteeringIncrement = 0.005f; // at high speeds (radians per 1/60 of a second)
    mutable float maxSteeringIncrement = 0.15f; // at low speeds (radians per 1/60 of a second)

    /* Used only to calculate steering increments, should be set roughly to a bit lower than max speed */
    /* The speed at which steering increment is equal to minSteeringIncrement */
    mutable float minSteeringAtSpeed = 140.0f;

    mutable btVector3 wheelDirectionCS0 = {0, -1, 0};
    mutable btVector3 wheelAxleCS = {-1, 0, 0};

    mutable std::vector<WheelPlacement> wheels = {
        {{-0.8f, -0.22f + centerOfMassOffset, 1.6f}, true}, // Front Left
        {{0.8f, -0.22f + centerOfMassOffset, 1.6f}, true}, // Front Right
        {{-0.8f, -0.22f + centerOfMassOffset, -1.32f}, false}, // Rear Left
        {{0.8f, -0.22f + centerOfMassOffset, -1.32f}, false} // Rear Right
    };
};
