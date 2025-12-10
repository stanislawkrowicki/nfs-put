#pragma once

#include <cstdint>
#include <LinearMath/btTransform.h>

struct __attribute__((packed)) PlayerPosition {
    uint16_t clientId;
    char position[sizeof(btTransformFloatData)];
};
