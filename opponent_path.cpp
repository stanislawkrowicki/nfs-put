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
    const auto pos = vehicle->getPosition();
    std::cout << "Added waypoint " << pos.x << " " << pos.y << " " << pos.z << std::endl;
    waypoints.push_back(pos);
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

std::vector<glm::vec3> OpponentPathGenerator::getRandomPathFromFile(const std::string &filename) {
    using json = nlohmann::json;
    std::vector<glm::vec3> path;

    const std::string file = std::string(SOURCE_PATH) + "/" + filename;

    if (!std::filesystem::exists(file)) {
        std::cerr << "getRandomPathFromFile: File does not exist: " << filename << '\n';
        return path;
    }

    std::ifstream infile(file);
    if (!infile) {
        std::cerr << "getRandomPathFromFile: Failed to open file: " << filename << '\n';
        return path;
    }

    json data;
    try {
        infile >> data;
    } catch (const std::exception &e) {
        std::cerr << "getRandomPathFromFile: Failed to parse JSON: " << e.what() << '\n';
        return path;
    }

    if (!data.contains("paths") || !data["paths"].is_array() || data["paths"].empty()) {
        std::cerr << "getRandomPathFromFile: No valid paths found in file.\n";
        return path;
    }

    // Pick a random path
    size_t randomIndex = rand() % data["paths"].size();
    const auto &jsonPath = data["paths"][randomIndex];

    if (!jsonPath.is_array()) {
        std::cerr << "getRandomPathFromFile: Selected path is not an array.\n";
        return path;
    }

    for (const auto &wp: jsonPath) {
        if (wp.is_array() && wp.size() == 3) {
            std::cout << "Adding waypoint " << wp[0].get<float>() << " " << wp[1].get<float>() << " " << wp[2].get<
                float>() << std::endl;
            path.emplace_back(wp[0].get<float>(), wp[1].get<float>(), wp[2].get<float>());
        } else {
            std::cerr << "getRandomPathFromFile: Malformed waypoint found.\n";
        }
    }

    return path;
}
