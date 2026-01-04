#pragma once
#include <functional>

class HandshakeAckHandler {
public:
    static void handle(const std::function<void(void)> &onAck) {
        onAck();
        std::cout << "Handshake successful!" << std::endl;
    }
};
