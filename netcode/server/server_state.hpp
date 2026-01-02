#pragma once
#include <mutex>
#include <condition_variable>

enum class MatchPhase {
    Lobby,
    Running,
    Finished
};

struct ServerState {
    std::mutex mtx;
    std::condition_variable cv;
    MatchPhase phase = MatchPhase::Lobby;
    void endMatch() {
        {
            std::lock_guard<std::mutex> lock(mtx);
            phase = MatchPhase::Finished;
        }
        cv.notify_all();
    }
};