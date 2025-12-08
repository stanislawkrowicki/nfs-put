#include <cstdint>
#include "crc32.hpp"

uint32_t *CRC32::getTable() {
    static uint32_t table[256];
    static bool initialized = false;

    if (!initialized) {
        for (uint32_t i = 0; i < 256; ++i) {
            uint32_t crc = i;
            for (int j = 0; j < 8; ++j) {
                if (crc & 1) {
                    crc = (crc >> 1) ^ CRC32_POLYNOMIAL;
                } else {
                    crc = (crc >> 1);
                }
            }
            table[i] = crc;
        }

        initialized = true;
    }

    return table;
}

uint32_t CRC32::calculate(const char *data, const size_t length) {
    const uint32_t *table = getTable();

    uint32_t crc = 0xFFFFFFFF;

    for (size_t i = 0; i < length; ++i) {
        crc = (crc >> 8) ^ table[(crc ^ data[i]) & 0xFF];
    }

    return ~crc;
}
