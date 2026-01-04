#pragma once

class HUD {
    static void drawCurrentLap();

    static void drawLeaderboard();

public:
    static void drawCountdown(int seconds);

    static void drawLapsOverlay();
};
