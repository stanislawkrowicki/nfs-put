#pragma once

#include <cstdint>

typedef uint8_t ClientInputs;

#define INPUT_THROTTLE (1 << 0)
#define INPUT_BRAKE (1 << 1)
#define INPUT_HANDBRAKE (1 << 2)
#define INPUT_LEFT (1 << 3)
#define INPUT_RIGHT (1 << 4)

inline ClientInputs buildInputBitmap(const bool left, const bool right, const bool handbrake, const bool forward,
                                     const bool backward) {
    uint8_t result = 0;

    if (left)
        result |= INPUT_LEFT;
    if (right)
        result |= INPUT_RIGHT;
    if (handbrake)
        result |= INPUT_HANDBRAKE;
    if (forward)
        result |= INPUT_THROTTLE;
    if (backward)
        result |= INPUT_BRAKE;

    return result;
}
