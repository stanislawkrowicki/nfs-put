#pragma once
#include <memory>
#include <vector>
#include "glm/glm.hpp"
#include <string>
#include "vehicle.hpp"

class OpponentPathGenerator {
    std::vector<glm::vec3> waypoints;

public:
    void clearWaypoints();

    void addWaypoint(const glm::vec3 &waypoint);

    void addWaypointFromVehicle(const std::shared_ptr<Vehicle> &vehicle);

    std::vector<glm::vec3> getWaypoints();

    void saveWaypointsToFile(const std::string &filename);

    static std::vector<glm::vec3> getRandomPathFromFile(const std::string &filename);
};

class OpponentPath {
};
