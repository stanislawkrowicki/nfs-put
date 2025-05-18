#pragma once
#include <memory>

#include "vehicle.hpp"

class VehicleManager {
public:
    static VehicleManager &getInstance() {
        static VehicleManager instance;
        return instance;
    }

    /* Singleton safety */
    VehicleManager(const VehicleManager &) = delete;

    VehicleManager &operator=(const VehicleManager &) = delete;

    VehicleManager(VehicleManager &&) = delete;

    VehicleManager &operator=(VehicleManager &&) = delete;


    std::shared_ptr<Vehicle> createVehicle(const VehicleConfig &config) {
        const auto vehicle = std::make_shared<Vehicle>(config);
        vehicles.push_back(vehicle);
        vehicle->addToWorld();
        return vehicle;
    }

    [[nodiscard]]
    const std::vector<std::shared_ptr<Vehicle> > &getVehicles() const {
        return vehicles;
    }

private:
    VehicleManager() = default;

    std::vector<std::shared_ptr<Vehicle> > vehicles;
};
