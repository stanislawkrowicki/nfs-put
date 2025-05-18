#include "physics.hpp"

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
}

void Physics::stepSimulation(const btScalar timeStep) const {
    dynamicsWorld->stepSimulation(timeStep);
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

// btRigidBody *Physics::getCarChassis() const {
//     return carChassis;
// }

DebugDrawer *Physics::getDebugDrawer() const {
    return debugDrawer;
}

btDynamicsWorld *Physics::getDynamicsWorld() const {
    return dynamicsWorld;
}
