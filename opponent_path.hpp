#pragma once
#include <memory>
#include <vector>
#include "glm/glm.hpp"
#include <string>
#include "vehicle.hpp"

class OpponentPathGenerator {
    std::vector<glm::vec3> waypoints;
    std::vector<std::vector<glm::vec3> > paths;

    OpponentPathGenerator() = default;

public:
    /* Singleton safety */
    OpponentPathGenerator(const OpponentPathGenerator &) = delete;

    OpponentPathGenerator &operator=(const OpponentPathGenerator &) = delete;

    OpponentPathGenerator(OpponentPathGenerator &&) = delete;

    OpponentPathGenerator &operator=(OpponentPathGenerator &&) = delete;

    static OpponentPathGenerator &getInstance();

    void clearWaypoints();

    void addWaypoint(const glm::vec3 &waypoint);

    void addWaypointFromVehicle(const std::shared_ptr<Vehicle> &vehicle);

    std::vector<glm::vec3> getWaypoints();

    void saveWaypointsToFile(const std::string &filename);

    void loadPathsToMemory(const std::string &filename);

    std::vector<glm::vec3> getRandomPathFromMemory();

    static std::vector<glm::vec3> getRandomPathFromFile(const std::string &filename);
};

class OpponentPath {
};
