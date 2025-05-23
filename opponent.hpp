#pragma once

#include "vehicle.hpp"

constexpr float WAYPOINT_THRESHOLD = 3.0f;

class Opponent {
    std::shared_ptr<Vehicle> vehicle;

    unsigned int currentWaypoint = 0;

    static glm::vec3 bezierPoint(float t, const glm::vec3 &p0, const glm::vec3 &p1, const glm::vec3 &p2);

public:
    // explicit Opponent(const std::shared_ptr<Vehicle> &vehicle);
    std::vector<glm::vec3> waypoints;

    Opponent(const std::shared_ptr<Vehicle> &vehicle, const std::vector<glm::vec3> &waypoints);

    void updateSteering();
};
