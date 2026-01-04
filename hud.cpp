#include "hud.hpp"
#include "imgui.h"
#include "laps.hpp"

void HUD::drawCurrentLap() {
    ImGui::Text("LAP: %d", Laps::getInstance().getLocalPlayerLaps());
}

void HUD::drawLeaderboard() {
    const auto &lapsManager = Laps::getInstance();
    const auto leaderboardEntries = lapsManager.getLeaderboard();

    for (const auto &[playerId, playerName, lapCount, position]: leaderboardEntries) {
        if (playerId == Laps::LOCAL_PLAYER_ID)
            ImGui::TextColored(ImVec4(0.0f, 0.7f, 0.7f, 1.0f), "%d. %s (%d laps)", position, playerName.c_str(),
                               lapCount);
        else
            ImGui::Text("%d. %s (%d laps)", position, playerName.c_str(), lapCount);
    }
}

void HUD::drawLapsOverlay() {
    constexpr float margin = 20.0f;

    ImGui::SetNextWindowPos(ImVec2(margin, margin), ImGuiCond_Always);

    constexpr ImGuiWindowFlags windowFlags = ImGuiWindowFlags_NoDecoration |
                                             ImGuiWindowFlags_AlwaysAutoResize |
                                             ImGuiWindowFlags_NoSavedSettings |
                                             ImGuiWindowFlags_NoFocusOnAppearing |
                                             ImGuiWindowFlags_NoNav |
                                             ImGuiWindowFlags_NoMove;
    ImGui::SetNextWindowBgAlpha(0.0f);

    if (ImGui::Begin("LAPS_Overlay", nullptr, windowFlags)) {
        ImGui::SetWindowFontScale(1.5);
        drawCurrentLap();
        ImGui::Separator();
        drawLeaderboard();
    }

    ImGui::End();
}

void HUD::drawCountdown(const int seconds) {
    if (seconds <= 0) return;

    const auto screenSize = ImGui::GetIO().DisplaySize;
    const auto center = ImVec2(screenSize.x * 0.5f, screenSize.y * 0.5f);

    ImGui::SetNextWindowPos(center, ImGuiCond_Always, ImVec2(0.5f, 0.5f));

    constexpr ImGuiWindowFlags flags = ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_AlwaysAutoResize |
                                       ImGuiWindowFlags_NoSavedSettings |
                                       ImGuiWindowFlags_NoInputs |
                                       ImGuiWindowFlags_NoBackground;

    if (ImGui::Begin("COUNTDOWN_Overlay", nullptr, flags)) {
        const float pulse = 1.0f + (sin(ImGui::GetTime() * 10.0f) * 0.1f);
        ImGui::SetWindowFontScale(10.0f * pulse);

        if (seconds == 1) {
            ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.0f, 1.0f), "%d", seconds);
        } else {
            ImGui::Text("%d", seconds);
        }
    }
    ImGui::End();
}
