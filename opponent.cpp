#include "opponent.hpp"
#include <iostream>

#include "glm/detail/_noise.hpp"

glm::vec3 Opponent::bezierPoint(const float t, const glm::vec3 &p0, const glm::vec3 &p1, const glm::vec3 &p2) {
    const float u = 1.0f - t;
    return u * u * p0 + 2.0f * u * t * p1 + t * t * p2;
}

Opponent::Opponent(const std::shared_ptr<Vehicle> &vehicle, const std::vector<glm::vec3> &waypoints) : vehicle(vehicle),
    waypoints(waypoints) {
}

glm::vec3 Opponent::findLookaheadPoint(const glm::vec3 &currentPos) const {
    unsigned int startAt = currentWaypoint;
    if (currentWaypoint == waypoints.size() - 1)
        startAt = 0;

    for (size_t i = 0; i < LOOKAHEAD_WAYPOINTS_COUNT; ++i) {
        const size_t idx1 = (startAt + i) % waypoints.size();
        const size_t idx2 = (startAt + i + 1) % waypoints.size();

        const glm::vec3 p1 = waypoints[idx1];
        const glm::vec3 p2 = waypoints[idx2];
        const glm::vec3 d = p2 - p1;
        const glm::vec3 f = p1 - currentPos;

        const float a = glm::dot(d, d);
        const float b = 2.0f * glm::dot(f, d);
        const float c = glm::dot(f, f) - LOOKAHEAD_DISTANCE * LOOKAHEAD_DISTANCE;

        float discriminant = b * b - 4.0f * a * c;
        if (discriminant < 0) continue;

        discriminant = sqrt(discriminant);

        const float t1 = (-b - discriminant) / (2.0f * a);
        const float t2 = (-b + discriminant) / (2.0f * a);

        if (t1 >= 0.0f && t1 <= 1.0f)
            return p1 + d * t1;
        if (t2 >= 0.0f && t2 <= 1.0f)
            return p1 + d * t2;
    }

    /* Fallback to the waypoint if didn't find any points inside the radius */
    /* probably should use Bézier curve along next 2 waypoints here? */
    return waypoints[currentWaypoint];
}

void Opponent::updateSteering() {
    const auto modelMatrix = vehicle->getOpenGLModelMatrix();
    const auto currentPos = glm::vec3(modelMatrix[3]);
    const glm::vec3 forward = glm::normalize(glm::vec3(modelMatrix[2])); // usually Z axis

    const glm::vec3 toTarget = waypoints[currentWaypoint] - currentPos;
    const float distance = glm::length(toTarget);

    /* We should probably be able to skip a waypoint if the AI is already ahead,
     * e.g. because of driving through grass */
    if (distance < WAYPOINT_THRESHOLD) {
        currentWaypoint = (currentWaypoint + 1) % waypoints.size();
    }

    const glm::vec3 lookahead = findLookaheadPoint(currentPos);

    glm::vec3 toLookahead = glm::normalize(lookahead - currentPos);
    float dot = glm::clamp(glm::dot(forward, toLookahead), -1.0f, 1.0f);
    float steeringAngle = acos(dot);

    float cross = glm::cross(forward, toLookahead).y;
    if (cross < 0) steeringAngle = -steeringAngle;

    glm::vec3 p0 = currentPos;
    glm::vec3 p1 = waypoints[(currentWaypoint + 1) % waypoints.size()];
    glm::vec3 p2 = waypoints[(currentWaypoint + 2) % waypoints.size()];

    glm::vec3 a = bezierPoint(0.0f, p0, p1, p2);
    glm::vec3 b = bezierPoint(0.5f, p0, p1, p2);
    glm::vec3 c = bezierPoint(1.0f, p0, p1, p2);

    glm::vec3 dir1 = glm::normalize(b - a);
    glm::vec3 dir2 = glm::normalize(c - b);

    float turnSharpness = acos(glm::clamp(glm::dot(dir1, dir2), -1.0f, 1.0f));

    /* The bigger the value the harder and earlier the AI will brake before a turn */
    constexpr float SHARPNESS_MULTIPLIER = 3.0f;

    const auto currentSpeed = abs(vehicle->getBtVehicle()->getCurrentSpeedKmHour());
    const auto expectedSpeed = std::max(50.0f, 200 * expf(-SHARPNESS_MULTIPLIER * turnSharpness));

    bool forwardThrottle = true;
    bool brake = false;

    if (currentSpeed > expectedSpeed) {
        brake = true;
        forwardThrottle = false;
    }

    vehicle->aiUpdateControls(forwardThrottle, brake, steeringAngle);
}
