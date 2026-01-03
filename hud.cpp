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

void HUD::draw() {
    constexpr float margin = 20.0f;

    ImGui::SetNextWindowPos(ImVec2(margin, margin), ImGuiCond_Always);

    constexpr ImGuiWindowFlags windowFlags = ImGuiWindowFlags_NoDecoration |
                                             ImGuiWindowFlags_AlwaysAutoResize |
                                             ImGuiWindowFlags_NoSavedSettings |
                                             ImGuiWindowFlags_NoFocusOnAppearing |
                                             ImGuiWindowFlags_NoNav |
                                             ImGuiWindowFlags_NoMove;
    ImGui::SetNextWindowBgAlpha(0.0f);

    if (ImGui::Begin("HUD_Overlay", nullptr, windowFlags)) {
        ImGui::SetWindowFontScale(1.5);
        drawCurrentLap();
        ImGui::Separator();
        drawLeaderboard();
    }

    ImGui::End();
}
