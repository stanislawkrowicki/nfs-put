#pragma once

#include <cstdint>
#include <cstddef>

class CRC32 {
    static constexpr uint32_t CRC32_POLYNOMIAL = 0xEDB88320;

    static uint32_t *getTable();

public:
    static uint32_t calculate(const char *data, size_t length);
};
