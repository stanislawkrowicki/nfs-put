#include "opponent.hpp"
#include <iostream>
#include <random>

#include "opponent_path.hpp"
#include "glm/detail/_noise.hpp"

glm::vec3 Opponent::bezierPoint(const float t, const glm::vec3 &p0, const glm::vec3 &p1, const glm::vec3 &p2) {
    const float u = 1.0f - t;
    return u * u * p0 + 2.0f * u * t * p1 + t * t * p2;
}

Opponent::Opponent(const std::shared_ptr<Vehicle> &vehicle) : vehicle(vehicle) {
    waypoints = OpponentPathGenerator::getInstance().getRandomPathFromMemory();
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

void Opponent::randomizePath() {
    constexpr float PATH_CHANGE_PROBABILITY = 0.1f;

    static std::random_device rd;
    static std::mt19937 gen(rd());
    static std::uniform_real_distribution dist(0.0f, 1.0f);

    if (dist(gen) > PATH_CHANGE_PROBABILITY)
        return;

    const auto path = OpponentPathGenerator::getRandomPathFromFile("paths.json");

    const glm::vec3 newCurrWaypoint = path[currentWaypoint];
    const auto modelMatrix = vehicle->getOpenGLModelMatrix();
    const auto currentPos = glm::vec3(modelMatrix[3]);
    const auto currentForward = glm::normalize(glm::vec3(modelMatrix[2]));

    const auto toNewWaypoint = glm::normalize(newCurrWaypoint - currentPos);
    const float dot = glm::clamp(glm::dot(currentForward, toNewWaypoint), -1.0f, 1.0f);

    unsigned int closestWaypoint = 0;

    std::cout << "DOT " << dot << std::endl;
    if (dot >= 0.0f)
        closestWaypoint = seekClosestSmallerWaypoint(path);
    else
        closestWaypoint = seekClosestBiggerWaypoint(path);

    std::cout << currentWaypoint << "  " << closestWaypoint << std::endl;
    waypoints = path;
    currentWaypoint = closestWaypoint;
}

unsigned int Opponent::seekClosestBiggerWaypoint(const std::vector<glm::vec3> &newPath) const {
    float lastDistance = -1;

    for (size_t i = 0; i < WAYPOINT_SEEK_DEPTH; ++i) {
        const auto newWaypointIndex = (currentWaypoint + i) % waypoints.size();
        const auto newWaypoint = newPath[newWaypointIndex];
        const auto currentPos = vehicle->getPosition();
        const auto distance = glm::distance(currentPos, newWaypoint);

        if (lastDistance < 0) {
            lastDistance = distance;
            continue;
        }

        std::cout << "DISTANCE  " << distance << "  " << lastDistance;
        if (distance > lastDistance)
            return (newWaypointIndex) % waypoints.size();

        lastDistance = distance;
    }

    /* Fallback */
    std::cout << "Fallback was called when seeking for closest waypoint forward" << std::endl;
    return (currentWaypoint + WAYPOINT_SEEK_DEPTH) % waypoints.size();
}

/* TODO: Implement minimum distance between changed waypoints */
unsigned int Opponent::seekClosestSmallerWaypoint(const std::vector<glm::vec3> &newPath) const {
    float lastDistance = -1;

    for (size_t i = 0; i < WAYPOINT_SEEK_DEPTH; ++i) {
        const auto newWaypointIndex = (currentWaypoint - i) % waypoints.size();
        const auto newWaypoint = newPath[newWaypointIndex];
        const auto currentPos = vehicle->getPosition();
        const auto distance = glm::distance(currentPos, newWaypoint);

        if (lastDistance < 0) {
            lastDistance = distance;
            continue;
        }

        /* +2 because we use a waypoint that's slightly further to avoid sharp turns */
        if (distance > lastDistance)
            return (newWaypointIndex + 2) % waypoints.size();

        lastDistance = distance;
    }

    /* Fallback */
    std::cout << "Fallback was called when seeking for closest waypoint backward" << std::endl;
    return (currentWaypoint - WAYPOINT_SEEK_DEPTH) % waypoints.size();
}

void Opponent::selectRandomNewPath() {
    waypoints = OpponentPathGenerator::getInstance().getRandomPathFromMemory();
    /* Set the waypoint to one ahead after the lap is finished
     * so that the change is not that jittery */
    currentWaypoint = 1;
}

void Opponent::updateSteering() {
    const auto modelMatrix = vehicle->getOpenGLModelMatrix();
    const auto currentPos = glm::vec3(modelMatrix[3]);
    const glm::vec3 forward = glm::normalize(glm::vec3(modelMatrix[2]));

    const glm::vec3 toTarget = waypoints[currentWaypoint] - currentPos;
    const float distance = glm::length(toTarget);

    /* We should probably be able to skip a waypoint if the AI is already ahead,
     * e.g. because of driving through grass */
    if (distance < WAYPOINT_THRESHOLD) {
        currentWaypoint = (currentWaypoint + 1) % waypoints.size();
        /* TODO: Implement random path changing across one lap.
         * Right now it selects the closest waypoint and switches the path,
         * but it's too aggressive. We need some interpolation */
        // randomizePath();
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

    if (currentWaypoint == waypoints.size() - 1)
        selectRandomNewPath();
}
