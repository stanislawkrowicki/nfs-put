#pragma once
#include <memory>

#include "physics_debug.hpp"
#include "model.hpp"
#include "btBulletDynamicsCommon.h"
#include "BulletDynamics/Vehicle/btRaycastVehicle.h"

class Physics {
    DebugDrawer *debugDrawer;
    btDefaultCollisionConfiguration *collisionConfig;
    btCollisionDispatcher *dispatcher;
    btBroadphaseInterface *broadphase;
    btSequentialImpulseConstraintSolver *solver;
    btDynamicsWorld *dynamicsWorld;

    btBvhTriangleMeshShape *groundShape{};
    btDefaultMotionState *groundMotion{};
    btRigidBody *groundRigidBody{};

    btBoxShape *chassisShape{};
    btDefaultMotionState *chassisMotion{};
    btDefaultVehicleRaycaster *raycaster{};
    btRaycastVehicle *vehicle{};
    btRigidBody *carChassis{};

    Physics();

public:
    static Physics &getInstance();

    /* Singleton safety */
    Physics(const Physics &) = delete;

    Physics &operator=(const Physics &) = delete;

    Physics(Physics &&) = delete;

    Physics &operator=(Physics &&) = delete;

    void initPhysics(const std::unique_ptr<btTriangleMesh> &triMesh);

    /* Not yet implemented.
     * Implement when we in need to reload maps */
    // void exitPhysics();

    void stepSimulation(btScalar timeStep) const;

    btRaycastVehicle *addVehicleToWorld();

    static std::unique_ptr<btTriangleMesh> btTriMeshFromModel(const std::vector<Vertex> &vertices,
                                                              const std::vector<unsigned int> &indices);

    [[nodiscard]]
    btRigidBody *getCarChassis() const;

    [[nodiscard]]
    DebugDrawer *getDebugDrawer() const;
};
