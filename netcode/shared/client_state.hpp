#pragma once

#include <cstdint>
#include "../shared/packets/udp/client/state_packet.hpp"

struct __attribute__((packed)) ClientState {
    uint16_t clientId;
    char state[STATE_PAYLOAD_SIZE];
};
