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
    mutable btVector3 chassisHalfExtents = {1, 0.5, 2};
    mutable btScalar chassisMargin = 0.02f;

    mutable btScalar mass = 800.0f;

    /* Suspension */
    mutable btScalar suspensionStiffness = 20;
    mutable btScalar suspensionRestLength = 0.6f;
    mutable btScalar dampingCompression = 4.4f;
    mutable btScalar dampingRelaxation = 2.3f;

    /* Tuning */
    mutable btRaycastVehicle::btVehicleTuning tuning;

    /* Wheels */
    mutable btScalar wheelRadius = 0.5f;
    mutable btScalar wheelWidth = 0.4f; // only for graphics
    mutable btScalar frictionSlip = 1000.0f;
    mutable btScalar rollInfluence = 0.1f;

    mutable btVector3 wheelDirectionCS0 = {0, -1, 0};
    mutable btVector3 wheelAxleCS = {1, 0, 0};

    mutable std::vector<WheelPlacement> wheels = {
        {{-1.5f, -0.5f, 1.5f}, true}, // Front Left
        {{1.5f, -0.5f, 1.5f}, true}, // Front Right
        {{-1.5f, -0.5f, -1.5f}, false}, // Rear Left
        {{1.5f, -0.5f, -1.5f}, false} // Rear Right
    };
};
