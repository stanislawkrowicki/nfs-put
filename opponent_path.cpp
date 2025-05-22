#include "opponent_path.hpp"
#include <iostream>
#include <fstream>
#include <nlohmann/json.hpp>

void OpponentPathGenerator::clearWaypoints() {
    waypoints.clear();
}

void OpponentPathGenerator::addWaypoint(const glm::vec3 &waypoint) {
    waypoints.push_back(waypoint);
}

void OpponentPathGenerator::addWaypointFromVehicle(const std::shared_ptr<Vehicle> &vehicle) {
    const auto pos = vehicle->getOpenGLModelMatrix()[0];
    std::cout << "Added waypoint " << pos.x << " " << pos.y << " " << pos.z << std::endl;
    waypoints.emplace_back(pos.x, pos.y, pos.z);
}

std::vector<glm::vec3> OpponentPathGenerator::getWaypoints() {
    return waypoints;
}

void OpponentPathGenerator::saveWaypointsToFile(const std::string &filename) {
    using json = nlohmann::json;
    json data;

    if (std::filesystem::exists(filename) && std::filesystem::file_size(filename) > 0) {
        std::ifstream infile(filename);
        try {
            infile >> data;
        } catch (const std::exception &e) {
            std::cerr << "Failed to parse existing JSON: " << e.what() << '\n';
        }
    }

    if (!data.contains("paths") || !data["paths"].is_array()) {
        data["paths"] = json::array();
    }

    json newPath = json::array();
    for (const auto &wp: waypoints) {
        newPath.push_back({wp.x, wp.y, wp.z});
    }

    data["paths"].push_back(newPath);

    std::ofstream outfile(std::string(SOURCE_PATH) + "/" + filename);
    if (!outfile) {
        std::cerr << "OpponentPathGenerator::Failed to open file for writing: " << filename << '\n';
        return;
    }

    outfile << data.dump() << std::endl;
    std::cout << "Appended new waypoint path to " << filename << std::endl;

    waypoints.clear();
}
