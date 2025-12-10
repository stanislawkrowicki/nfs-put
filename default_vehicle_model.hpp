#pragma once
#include <memory>
#include "model.hpp"

class VehicleModelCache {
public:
    static std::shared_ptr<Model> getDefaultVehicleModel() {
        static const auto vehicleModel = std::make_shared<Model>(
            "skyline.glb",
            true,
            aiProcess_Triangulate |
            aiProcess_PreTransformVertices |
            aiProcess_JoinIdenticalVertices |
            aiProcess_ImproveCacheLocality |
            aiProcess_GenSmoothNormals |
            aiProcess_SortByPType
        );
        return vehicleModel;
    }
};
