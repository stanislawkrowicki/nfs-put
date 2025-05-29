#pragma once

#include "vehicle.hpp"

constexpr float WAYPOINT_THRESHOLD = 5.0f;
constexpr float LOOKAHEAD_DISTANCE = 8.0f;

/* How many waypoints starting from current should the pure pursuit algorithm
 * try to find intersection at */
constexpr unsigned int LOOKAHEAD_WAYPOINTS_COUNT = 5;

/* How many waypoints to traverse when looking for the closest one */
constexpr unsigned int WAYPOINT_SEEK_DEPTH = 15;

class Opponent {
    std::shared_ptr<Vehicle> vehicle;

    unsigned int currentWaypoint = 0;

    static glm::vec3 bezierPoint(float t, const glm::vec3 &p0, const glm::vec3 &p1, const glm::vec3 &p2);

    [[nodiscard]]
    glm::vec3 findLookaheadPoint(const glm::vec3 &currentPos) const;

    /* With some random chance changes the path that the AI is following */
    void randomizePath();

    [[nodiscard]]
    unsigned int seekClosestBiggerWaypoint(const std::vector<glm::vec3> &newPath) const;

    [[nodiscard]]
    unsigned int seekClosestSmallerWaypoint(const std::vector<glm::vec3> &newPath) const;

    void selectRandomNewPath();

public:
    // explicit Opponent(const std::shared_ptr<Vehicle> &vehicle);
    std::vector<glm::vec3> waypoints;

    explicit Opponent(const std::shared_ptr<Vehicle> &vehicle);

    void updateSteering();
};

