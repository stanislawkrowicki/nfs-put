#include "opponent_path.hpp"
#include <iostream>
#include <fstream>
#include <random>
#include <nlohmann/json.hpp>

OpponentPathGenerator &OpponentPathGenerator::getInstance() {
    static OpponentPathGenerator instance;
    return instance;
}

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

    const std::string filepath = std::string(SOURCE_PATH) + "/" + filename;

    if (std::filesystem::exists(filepath) && std::filesystem::file_size(filepath) > 0) {
        std::ifstream infile(filepath);
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

    std::ofstream outfile(filepath);
    if (!outfile) {
        std::cerr << "OpponentPathGenerator::Failed to open file for writing: " << filepath << '\n';
        return;
    }

    outfile << data.dump() << std::endl;
    std::cout << "Appended new waypoint path to " << filepath << std::endl;

    waypoints.clear();
}

void OpponentPathGenerator::loadPathsToMemory(const std::string &filename) {
    using json = nlohmann::json;
    const std::string file = std::string(SOURCE_PATH) + "/" + filename;

    paths.clear();

    if (!std::filesystem::exists(file)) {
        std::cerr << "loadPathsToMemory: File does not exist: " << file << std::endl;
        return;
    }

    std::ifstream infile(file);
    if (!infile) {
        std::cerr << "loadPathsToMemory: Failed to open file: " << file << std::endl;
        return;
    }

    json data;
    try {
        infile >> data;
    } catch (const std::exception &e) {
        std::cerr << "loadPathsToMemory: Failed to parse JSON: " << e.what() << std::endl;
        return;
    }

    if (!data.contains("paths") || !data["paths"].is_array()) {
        std::cerr << "loadPathsToMemory: Invalid or missing 'paths' array in file." << std::endl;
        return;
    }

    for (const auto &jsonPath: data["paths"]) {
        if (!jsonPath.is_array()) {
            std::cerr << "loadPathsToMemory: Skipping invalid path (not an array)." << std::endl;;
            continue;
        }

        std::vector<glm::vec3> path;
        for (const auto &wp: jsonPath) {
            if (wp.is_array() && wp.size() == 3) {
                path.emplace_back(wp[0].get<float>(), wp[1].get<float>(), wp[2].get<float>());
            } else {
                std::cerr << "loadPathsToMemory: Malformed waypoint encountered." << std::endl;;
            }
        }

        if (!path.empty()) {
            paths.push_back(std::move(path));
        }
    }

    std::cout << "Loaded " << paths.size() << " paths into memory.\n";
}

std::vector<glm::vec3> OpponentPathGenerator::getRandomPathFromMemory() {
    if (paths.empty()) {
        std::cerr << "Tried to select random path from memory without loading first!" << std::endl;
        exit(1);
    }

    static std::random_device rd;
    static std::mt19937 gen(rd());
    static std::uniform_int_distribution<unsigned long> dist(0, paths.size() - 1);

    const size_t randomIndex = dist(gen);
    return paths[randomIndex];
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

    static std::random_device rd;
    static std::mt19937 gen(rd());
    static std::uniform_int_distribution<unsigned long> dist(0, data["paths"].size() - 1);

    size_t randomIndex = dist(gen);
    const auto &jsonPath = data["paths"][randomIndex];

    if (!jsonPath.is_array()) {
        std::cerr << "getRandomPathFromFile: Selected path is not an array.\n";
        return path;
    }

    for (const auto &wp: jsonPath) {
        if (wp.is_array() && wp.size() == 3) {
            path.emplace_back(wp[0].get<float>(), wp[1].get<float>(), wp[2].get<float>());
        } else {
            std::cerr << "getRandomPathFromFile: Malformed waypoint found.\n";
        }
    }

    return path;
}
