#pragma once
#include <stdexcept>

class DeserializationError final : public std::runtime_error {
public:
    explicit DeserializationError(const std::string &msg) : std::runtime_error(msg) {
    }
};
