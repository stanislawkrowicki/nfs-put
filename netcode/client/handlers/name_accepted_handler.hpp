#pragma once

#include <iostream>

class NameAcceptedHandler {
public:
    static void handle() {
        std::cout << "Name accepted!" << std::endl;
    }
};
