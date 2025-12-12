#pragma once

#include <cstdint>
#include <LinearMath/btTransform.h>

struct __attribute__((packed)) ClientState {
    uint16_t clientId;
    char transform[sizeof(btTransformFloatData)];
};
