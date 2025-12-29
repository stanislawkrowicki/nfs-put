#pragma once
#include <iostream>

class NameTakenHandler {
public:
    static void handle() {
        std::cout << "Name is already taken. Provide another one: " << std::flush;
    }
};
