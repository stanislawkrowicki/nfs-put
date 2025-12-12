#pragma once

#include <iostream>
#include <iomanip>
#include <chrono>

inline void printByteDump(const char *buffer, const size_t size) {
    if (buffer == nullptr || size == 0) {
        std::cerr << "Buffer is empty or null." << std::endl;
        return;
    }

    const auto currentTime = std::chrono::high_resolution_clock::now();
    std::cout << currentTime << " ";

    std::cout << std::hex << std::uppercase << std::setfill('0');

    for (size_t i = 0; i < size; ++i) {
        const auto byte_value = static_cast<unsigned int>(
            static_cast<uint8_t>(buffer[i])
        );
        std::cout << std::hex << byte_value << " ";
    }

    std::cout << std::dec << std::setfill(' ');
}
