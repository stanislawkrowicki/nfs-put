#pragma once

#include "LinearMath/btQuaternion.h"
#include "LinearMath/btVector3.h"

struct LapCheckpointPosition {
    btVector3 pos;
    btQuaternion rot;
    btVector3 dimensions;
};

inline LapCheckpointPosition LAP_CHECKPOINTS[5] = {
    LapCheckpointPosition{
        btVector3(-123.873, 6.80561, 3.65577), btQuaternion(-0.0630383, -0.698318, -0.032635, 0.71226),
        btVector3(20, 5, 2)
    },
    LapCheckpointPosition{
        btVector3(-235.471, 26.3858, -301.958), btQuaternion(-0.00402225, 0.967598, 0.0853639, -0.237593),
        btVector3(20, 5, 2)
    },
    LapCheckpointPosition{
        btVector3(9.9177, 31.8776, -291.096), btQuaternion(0.0457679, 0.578483, -0.0117274, 0.814325),
        btVector3(20, 5, 2)
    },
    LapCheckpointPosition{
        btVector3(93.4412, 18.1565, -134.336), btQuaternion(0.0111986, 0.610081, 0.0225576, 0.791938),
        btVector3(20, 5, 2)
    },
    LapCheckpointPosition{
        btVector3(-31.0629, -0.0636561, 5.14421), btQuaternion(-0.00656033, -0.717272, -0.0100661, 0.696689),
        btVector3(20, 5, 2)
    }
};
