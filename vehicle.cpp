#include "vehicle.hpp"

#include <iostream>
#include <utility>

void Vehicle::createBtVehicle() {
    chassisShape = new btBoxShape(config.chassisHalfExtents);

    btTransform chassisTransform;
    chassisTransform.setIdentity();
    chassisTransform.setOrigin(config.position);
    chassisTransform.setRotation(config.rotation);

    btVector3 inertia(0, 0, 0);
    chassisShape->calculateLocalInertia(config.mass, inertia);
    chassisMotion = new btDefaultMotionState(chassisTransform);
    const btRigidBody::btRigidBodyConstructionInfo carCI(config.mass, chassisMotion, chassisShape, inertia);
    chassis = new btRigidBody(carCI);

    const btRaycastVehicle::btVehicleTuning tuning;
    raycaster = new btDefaultVehicleRaycaster(dynamicsWorld);
    btVehicle = new btRaycastVehicle(tuning, chassis, raycaster);
    chassis->setActivationState(DISABLE_DEACTIVATION);

    const auto wheelDir = config.wheelDirectionCS0;
    const auto wheelAxle = config.wheelAxleCS;
    const auto suspensionRest = config.suspensionRestLength;
    const auto wheelRadius = config.wheelRadius;

    for (const auto [connectionPoint, isFrontWheel]: config.wheels) {
        btVehicle->addWheel(connectionPoint, wheelDir, wheelAxle, suspensionRest, wheelRadius, tuning,
                            isFrontWheel);
    }

    for (int i = 0; i < btVehicle->getNumWheels(); i++) {
        btWheelInfo &wheel = btVehicle->getWheelInfo(i);
        wheel.m_suspensionStiffness = config.suspensionStiffness;
        wheel.m_wheelsDampingRelaxation = config.dampingRelaxation;
        wheel.m_wheelsDampingCompression = config.dampingCompression;
        wheel.m_frictionSlip = config.frictionSlip;
        wheel.m_rollInfluence = config.rollInfluence;
    }
}

Vehicle::Vehicle(VehicleConfig config): config(std::move(config)) {
    dynamicsWorld = Physics::getInstance().getDynamicsWorld();
    createBtVehicle();
}

Vehicle::~Vehicle() {
    dynamicsWorld->removeAction(btVehicle);
    dynamicsWorld->removeRigidBody(btVehicle->getRigidBody());

    delete chassisShape;
    delete chassisMotion;
    delete chassis;
    delete raycaster;
    delete btVehicle;
}

void Vehicle::addToWorld() const {
    dynamicsWorld->addRigidBody(chassis);
    dynamicsWorld->addVehicle(btVehicle);
}

btRaycastVehicle *Vehicle::getBtVehicle() const {
    return btVehicle;
}

std::string Vehicle::getName() const {
    return config.name;
}

VehicleConfig Vehicle::getConfig() const {
    return config;
}

void Vehicle::updateControls(const bool forward, const bool backward, const bool handbrake, const bool left,
                             const bool right, const float dt) const {
    float appliedEngineForce = 0.0f;
    float appliedHandbrakeForce = 0.0f;

    if (forward)
        appliedEngineForce = config.engineForce;

    if (backward) {
        appliedEngineForce = -config.brakingForce;
    }

    if (handbrake)
        appliedHandbrakeForce = config.handbrakeForce;

    /* Technically set to front wheels, but feels like rear-wheel drive? */
    btVehicle->applyEngineForce(appliedEngineForce, 0); // Front left
    btVehicle->applyEngineForce(appliedEngineForce, 1); // Front right

    btVehicle->setBrake(appliedHandbrakeForce, 2); // Rear left
    btVehicle->setBrake(appliedHandbrakeForce, 3); // Rear right

    static float steering = 0.0f;

    /** See config.steeringIncrement comment */
    constexpr float steeringMultiplicationFactor = 60;

    const auto steeringDelta = config.steeringIncrement * steeringMultiplicationFactor * dt;

    if (left) {
        steering += steeringDelta;
        steering = std::min(steering, config.maxSteeringAngle);
    } else if (right) {
        steering -= steeringDelta;
        steering = std::max(steering, -config.maxSteeringAngle);
    } else {
        steering *= 0.9f;
        if (steering < 0.05f)
            steering = 0;
    }

    btVehicle->setSteeringValue(steering, 0); // Front left
    btVehicle->setSteeringValue(steering, 1); // Front right;
}

