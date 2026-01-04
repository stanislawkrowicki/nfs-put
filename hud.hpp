#pragma once
#include <string>

#include "imgui.h"
#include "glm/vec3.hpp"

class HUD {
    static void drawCurrentLap();

    static void drawLeaderboard();

public:
    static void begin(float currentWindowWidth, float currentWindowHeight);

    static void render();

    static void drawOpponentName(const std::string &nick, const ImVec2 &screenPos, float distance,
                                 float currentWindowHeight);

    static void drawCountdown(int seconds);

    static void drawLapsOverlay();
};
