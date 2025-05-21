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
    mutable btScalar chassisMargin = 0.02f;

    mutable btScalar mass = 800.0f;

    /* Forces */
    mutable float engineForce = 3000.0f;
    mutable float brakingForce = 3000.0f; // used as engineForce to simplify
    mutable float handbrakeForce = 100.0f;

    /* Suspension */
    mutable btScalar suspensionStiffness = 20;
    mutable btScalar suspensionRestLength = 0.2f;
    mutable btScalar dampingCompression = 4.4f;
    mutable btScalar dampingRelaxation = 2.3f;

    /* Tuning */
    mutable btRaycastVehicle::btVehicleTuning tuning;

    /* Wheels */
    mutable btScalar wheelRadius = 0.7f;
    mutable btScalar wheelWidth = 0.6f; // only for graphics
    mutable btScalar frictionSlip = 1.2f;
    mutable btScalar rollInfluence = 1.0f;

    mutable float maxSteeringAngle = 0.7f; // radians
    mutable float steeringIncrement = 0.04f; // radians per 1/60 of a second

    mutable btVector3 wheelDirectionCS0 = {0, -1, 0};
    mutable btVector3 wheelAxleCS = {-1, 0, 0};

    mutable std::vector<WheelPlacement> wheels = {
        {{-1.0f, 0.0f, 1.6f}, true}, // Front Left
        {{1.0f, 0.0f, 1.6f}, true}, // Front Right
        {{-1.0f, 0.0f, -1.2f}, false}, // Rear Left
        {{1.0f, 0.0f, -1.2f}, false} // Rear Right
    };
};
