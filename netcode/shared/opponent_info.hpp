#pragma once

#include <string>
#include <cstdint>

struct __attribute__((packed)) PlayerVehicleColor {
    uint8_t r;
    uint8_t g;
    uint8_t b;

    [[nodiscard]] float rNormalized() const { return static_cast<float>(r) / 255.0f; };
    [[nodiscard]] float gNormalized() const { return static_cast<float>(g) / 255.0f; };
    [[nodiscard]] float bNormalized() const { return static_cast<float>(b) / 255.0f; };
};

struct OpponentInfo {
    uint16_t id;
    PlayerVehicleColor vehicleColor;
    uint8_t gridPosition;
    std::string nickname;
};
