#include "vehicle.hpp"

#include <iostream>
#include <utility>

#include "glm/gtc/type_ptr.hpp"

void Vehicle::createBtVehicle() {
    auto boxShape = new btBoxShape(config.chassisHalfExtents);

    chassisShape = new btCompoundShape();

    btTransform massTransform;
    massTransform.setIdentity();
    /* By raising the boxShape child up we lower the center of mass
     * (the boxShape is higher  but center of mass stays the same) */
    massTransform.setOrigin(btVector3(0, config.centerOfMassOffset, 0));

    chassisShape->addChildShape(massTransform, boxShape);

    /* This part is taken from Bullet example.
     * I don't know what it's supposed to do,
     * but I feel like the car has more grip at the rear
     * even though this support shape is on the front? */
    const auto suppShape = new btBoxShape(btVector3(0.5f, 0.1f, 0.5f));
    btTransform suppLocalTrans;
    suppLocalTrans.setIdentity();
    suppLocalTrans.setOrigin(btVector3(0, 1.0, 2.0));
    chassisShape->addChildShape(suppLocalTrans, suppShape);

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

float Vehicle::calculateSteeringIncrement(const float speed) const {
    return std::clamp(
        config.maxSteeringIncrement - (config.maxSteeringIncrement * (speed / config.minSteeringAtSpeed) * 0.9f),
        config.minSteeringIncrement,
        config.maxSteeringIncrement);
}

float Vehicle::getEngineBoost() const {
    const float normalizedSpeed = glm::clamp(std::abs(btVehicle->getCurrentSpeedKmHour()) / config.boostMaxSpeed,
                                             0.0f, 1.0f);
    const float boost = config.boostStrength * expf(-config.boostFalloffRate * normalizedSpeed);

    return 1.0f + boost;
}

Vehicle::Vehicle(VehicleConfig config, std::shared_ptr<Model> vehicleModel): config(std::move(config)) {
    dynamicsWorld = Physics::getInstance().getDynamicsWorld();
    model = std::move(vehicleModel);
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

btRigidBody *Vehicle::getBtChassis() const {
    return chassis;
}

std::string Vehicle::getName() const {
    return config.name;
}

VehicleConfig Vehicle::getConfig() const {
    return config;
}

std::shared_ptr<Model> Vehicle::getModel() const {
    return model;
}

glm::mat4 Vehicle::getOpenGLModelMatrix() const {
    btScalar btMatrix[16];
    btVehicle->getChassisWorldTransform().getOpenGLMatrix(btMatrix);
    glm::mat4 modelMatrix = glm::make_mat4(btMatrix);
    modelMatrix = glm::translate(modelMatrix, glm::vec3(0.0f, config.centerOfMassOffset, 0.0f));
    return modelMatrix;
}

glm::vec3 Vehicle::getPosition() const {
    const auto pos = getOpenGLModelMatrix()[3];
    return {pos.x, pos.y, pos.z};
}

bool Vehicle::getIsBraking() const {
    return isBraking;
}

void Vehicle::updateControls(const bool forward, const bool backward, const bool handbrake, const bool left,
                             const bool right, const float dt) {
    float appliedEngineForce = 0.0f;
    float appliedHandbrakeForce = 0.0f;

    isBraking = backward || handbrake;

    if (forward) {
        appliedEngineForce = config.engineForce;
        appliedEngineForce *= getEngineBoost();
    }
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

    float steering = lastSteering;

    /** See config.steeringIncrement comment */
    constexpr float steeringMultiplicationFactor = 60;

    const auto currentSpeed = abs(btVehicle->getCurrentSpeedKmHour());
    const auto steeringDelta = calculateSteeringIncrement(currentSpeed) * steeringMultiplicationFactor * dt;

    if (left) {
        steering += steeringDelta;
        steering = std::min(steering, config.maxSteeringAngle);
    } else if (right) {
        steering -= steeringDelta;
        steering = std::max(steering, -config.maxSteeringAngle);
    } else {
        const float strength = std::clamp(10.0f + 0.2f * currentSpeed, 10.0f, 30.0f);
        steering *= std::exp(-strength * dt);

        if (std::abs(steering) < 0.01f)
            steering = 0;
    }

    btVehicle->setSteeringValue(steering, 0); // Front left
    btVehicle->setSteeringValue(steering, 1); // Front right;

    lastSteering = steering;
}

void Vehicle::aiUpdateControls(const bool forward, const bool backward, const float steering) {
    float appliedEngineForce = 0.0f;
    float appliedBrakeForce = 0.0f;

    isBraking = backward;

    if (forward) {
        appliedEngineForce = config.engineForce;
        appliedEngineForce *= getEngineBoost();
    }

    if (backward) {
        appliedEngineForce = -config.brakingForce;
    }

    btVehicle->applyEngineForce(appliedEngineForce, 0);
    btVehicle->applyEngineForce(appliedEngineForce, 1);
    btVehicle->applyEngineForce(appliedEngineForce, 2);
    btVehicle->applyEngineForce(appliedEngineForce, 3);

    // btVehicle->setBrake(appliedBrakeForce, 0);
    // btVehicle->setBrake(appliedBrakeForce, 1);
    // btVehicle->setBrake(appliedBrakeForce, 2);
    // btVehicle->setBrake(appliedBrakeForce, 3);

    float clampedSteering = steering;
    if (std::abs(clampedSteering) < 0.01f)
        clampedSteering = 0.0f;
    else
        clampedSteering = std::clamp(steering, -config.maxSteeringAngle, config.maxSteeringAngle);

    btVehicle->setSteeringValue(clampedSteering, 0);
    btVehicle->setSteeringValue(clampedSteering, 1);
}

float Vehicle::applyRotationToWheel(const size_t wheelIndex, const float deltaRotation) {
    return wheelRollingRotation[wheelIndex] += deltaRotation;
}

void Vehicle::printDebugPosition() const {
    const auto transform = this->btVehicle->getChassisWorldTransform();
    const auto pos = transform.getOrigin();
    const auto rot = transform.getRotation();

    std::cout << pos.getX() << " " << pos.getY() << " " << pos.getZ() << " | " << rot.getX() << " " << rot.getY() << " "
            << rot.getZ() << " " << rot.getW() << std::endl;
}
