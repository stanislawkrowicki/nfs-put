#pragma once

#include <string>
#include <vector>
#include <sys/epoll.h>
#include <thread>
#include <atomic>
#include <condition_variable>
struct ClientState {
    std::mutex mtx;
    std::condition_variable cv;
    bool ready = false;
};
class TCPClient {
public:
    explicit TCPClient(std::shared_ptr<ClientState> state);
    ~TCPClient();

    void connect(const char* host, const char* port);
    void send(const char* data, ssize_t size) const;

private:
    int socketFd{-1};
    int epollFd{-1};

    mutable std::atomic<int> localTimeLeft{0};   // local countdown in seconds
    mutable std::thread countdownThread;         // background countdown thread
    mutable std::string lastLobbyMessage; // latest lobby + countdown from server

    [[noreturn]]
    void loop() const;

    void handleServerMessage() const;
    void handleUserInput() const;
    std::shared_ptr<ClientState> state;
};
