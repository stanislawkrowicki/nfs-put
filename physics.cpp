#include "physics.hpp"

#include <iostream>
#include <memory>

#include "model.hpp"

Physics::Physics() {
    collisionConfig = new btDefaultCollisionConfiguration();
    dispatcher = new btCollisionDispatcher(collisionConfig);
    broadphase = new btDbvtBroadphase();
    solver = new btSequentialImpulseConstraintSolver();
    dynamicsWorld = new btDiscreteDynamicsWorld(dispatcher, broadphase, solver, collisionConfig);
    debugDrawer = new DebugDrawer(dynamicsWorld);
}

Physics &Physics::getInstance() {
    static Physics instance;
    return instance;
}

void Physics::initPhysics(const std::unique_ptr<btTriangleMesh> &triMesh) {
    /* Add static map body to the world */
    constexpr bool useQuantizedAabbCompression = true;
    groundShape = new btBvhTriangleMeshShape(triMesh.get(), useQuantizedAabbCompression);
    groundShape->setMargin(0.02f);

    groundMotion = new btDefaultMotionState(btTransform::getIdentity());

    const btRigidBody::btRigidBodyConstructionInfo groundCI(0.0f, groundMotion, groundShape);
    groundRigidBody = new btRigidBody(groundCI);

    dynamicsWorld->addRigidBody(groundRigidBody);

    /* Config */
    dynamicsWorld->setGravity(btVector3(0, -9.81f, 0));

    /* Assign debug drawer */
    debugDrawer = new DebugDrawer(dynamicsWorld);
}

void Physics::stepSimulation(const btScalar timeStep) const {
    dynamicsWorld->stepSimulation(timeStep);
}

btRaycastVehicle *Physics::addVehicleToWorld() {
    chassisShape = new btBoxShape(btVector3(1, 0.5, 2));
    btTransform chassisTransform;
    chassisTransform.setIdentity();
    chassisTransform.setOrigin(btVector3(0, 2, 3));
    const btQuaternion rotation(btVector3(0, 1, 0), SIMD_HALF_PI);
    chassisTransform.setRotation(rotation);

    constexpr btScalar mass = 800;
    btVector3 inertia(0, 0, 0);
    chassisShape->calculateLocalInertia(mass, inertia);
    chassisMotion = new btDefaultMotionState(chassisTransform);
    const btRigidBody::btRigidBodyConstructionInfo carCI(mass, chassisMotion, chassisShape, inertia);
    carChassis = new btRigidBody(carCI);
    dynamicsWorld->addRigidBody(carChassis);

    const btRaycastVehicle::btVehicleTuning tuning;
    raycaster = new btDefaultVehicleRaycaster(dynamicsWorld);
    vehicle = new btRaycastVehicle(tuning, carChassis, raycaster);
    carChassis->setActivationState(DISABLE_DEACTIVATION);
    dynamicsWorld->addVehicle(vehicle);

    const btVector3 wheelDir(0, -1, 0);
    const btVector3 wheelAxle(1, 0, 0);
    constexpr float suspensionRest = 0.6f;
    constexpr float wheelRadius = 0.5f;

    vehicle->addWheel(btVector3(-1.5, -0.5, 1.5), wheelDir, wheelAxle, suspensionRest, wheelRadius, tuning, true);
    vehicle->addWheel(btVector3(1.5, -0.5, 1.5), wheelDir, wheelAxle, suspensionRest, wheelRadius, tuning, true);
    vehicle->addWheel(btVector3(-1.5, -0.5, -1.5), wheelDir, wheelAxle, suspensionRest, wheelRadius, tuning, false);
    vehicle->addWheel(btVector3(1.5, -0.5, -1.5), wheelDir, wheelAxle, suspensionRest, wheelRadius, tuning, false);

    for (int i = 0; i < vehicle->getNumWheels(); i++) {
        btWheelInfo &wheel = vehicle->getWheelInfo(i);
        wheel.m_suspensionStiffness = 20;
        wheel.m_wheelsDampingRelaxation = 2.3f;
        wheel.m_wheelsDampingCompression = 4.4f;
        wheel.m_frictionSlip = 1000;
        wheel.m_rollInfluence = 0.1;
    }

    return vehicle;
}

std::unique_ptr<btTriangleMesh> Physics::
btTriMeshFromModel(const std::vector<Vertex> &vertices, const std::vector<unsigned int> &indices) {
    auto triMesh = std::make_unique<btTriangleMesh>();

    for (size_t i = 0; i < indices.size(); i += 3) {
        const Vertex &v0 = vertices[indices[i]];
        const Vertex &v1 = vertices[indices[i + 1]];
        const Vertex &v2 = vertices[indices[i + 2]];

        btVector3 bv0(v0.Position.x, v0.Position.y, v0.Position.z);
        btVector3 bv1(v1.Position.x, v1.Position.y, v1.Position.z);
        btVector3 bv2(v2.Position.x, v2.Position.y, v2.Position.z);

        triMesh->addTriangle(bv0, bv1, bv2);
    }

    return triMesh;
}

btRigidBody *Physics::getCarChassis() const {
    return carChassis;
}

DebugDrawer *Physics::getDebugDrawer() const {
    return debugDrawer;
}

