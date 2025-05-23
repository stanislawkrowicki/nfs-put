#include "opponent.hpp"
#include <iostream>

// Opponent::Opponent(const std::shared_ptr<Vehicle> &vehicle) : vehicle(vehicle) {
// }

glm::vec3 Opponent::bezierPoint(const float t, const glm::vec3 &p0, const glm::vec3 &p1, const glm::vec3 &p2) {
    float u = 1.0f - t;
    return u * u * p0 + 2.0f * u * t * p1 + t * t * p2;
}

Opponent::Opponent(const std::shared_ptr<Vehicle> &vehicle, const std::vector<glm::vec3> &waypoints) : vehicle(vehicle),
    waypoints(waypoints) {
}

void Opponent::updateSteering() {
    const auto modelMatrix = vehicle->getOpenGLModelMatrix();
    const auto currentPos = glm::vec3(modelMatrix[3]);
    const auto currentDirection = glm::vec3(modelMatrix[2]);

    const glm::vec3 toTarget = waypoints[currentWaypoint] - currentPos;
    const glm::vec3 targetDirection = glm::normalize(toTarget);

    const float distance = glm::length(toTarget);

    // std::cout << distance << std::endl;

    if (distance < WAYPOINT_THRESHOLD) {
        currentWaypoint = (currentWaypoint + 1) % waypoints.size();
        // std::cerr << "Next waypoint" << currentWaypoint << std::endl;
    }

    const float dot = glm::dot(currentDirection, targetDirection);
    const float angle = acos(dot);
    const float cross = glm::cross(currentDirection, targetDirection).y;

    bool forward = false;
    bool backward = false;
    bool left = false;
    bool right = false;

    if (angle > 0.15f) {
        if (cross > 0) left = true;
        else right = true;
    }

    glm::vec3 p0 = currentPos;
    glm::vec3 p1 = waypoints[(currentWaypoint + 1) % waypoints.size()];
    glm::vec3 p2 = waypoints[(currentWaypoint + 2) % waypoints.size()];

    glm::vec3 a = bezierPoint(0.0f, p0, p1, p2);
    glm::vec3 b = bezierPoint(0.5f, p0, p1, p2);
    glm::vec3 c = bezierPoint(1.0f, p0, p1, p2);

    glm::vec3 dir1 = glm::normalize(b - a);
    glm::vec3 dir2 = glm::normalize(c - b);

    float turnSharpness = acos(glm::clamp(glm::dot(dir1, dir2), -1.0f, 1.0f));

    const auto currentSpeed = abs(vehicle->getBtVehicle()->getCurrentSpeedKmHour());
    const auto expectedSpeed = std::max(50.0f, 200 * expf(-4.0f * turnSharpness));

    std::cout << turnSharpness << "   " << expectedSpeed << "   " << currentSpeed << std::endl;
    if (currentSpeed > expectedSpeed) {
        backward = true;
        std::cout << "BRAKING!!" << std::endl;
    } else forward = true;

    vehicle->aiUpdateControls(forward, backward, left, right);
}
