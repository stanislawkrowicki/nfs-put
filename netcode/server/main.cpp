#include <iostream>
#include <sys/socket.h>
#include <netdb.h>
#include <cstring>
#include <thread>
#include <condition_variable>
#include <csignal>

#include "loop.hpp"
#include "tcp_server.hpp"
#include "udp_server.hpp"

std::shared_ptr<ClientManager> clientManager;
std::shared_ptr<UDPServer> udpServer;
std::shared_ptr<TCPServer> tcpServer;
std::shared_ptr<ServerState> state;

std::thread udpServerThread;
std::thread tcpServerThread;
std::thread gameThread;

std::atomic shouldStop{false};

void handleSigint(int signum) {
    shouldStop = true;

    if (state)
        state->cv.notify_all();
}

void shutdown() {
    state->phase = MatchPhase::Finished;
    Loop::stop();

    udpServer->stopListening();
    if (udpServerThread.joinable()) udpServerThread.join();

    tcpServer->cleanLobbyTimerThread();
    tcpServer->stopListening();
    if (tcpServerThread.joinable()) tcpServerThread.join();
}

int main(const int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <port>", argv[0]);
        return 1;
    }

    std::signal(SIGINT, handleSigint);

    try {
        clientManager = std::make_shared<ClientManager>();
        udpServer = std::make_shared<UDPServer>(clientManager);

        state = std::make_shared<ServerState>();
        tcpServer = std::make_shared<TCPServer>(clientManager, state);

        udpServer->setTcpBridge(tcpServer);

        udpServer->bind(argv[1]);
        tcpServer->listen(argv[1]);

        udpServerThread = std::thread([&] {
            udpServer->loop();
        });

        tcpServerThread = std::thread([&] {
            tcpServer->loop();
        });
    } catch (std::runtime_error &e) {
        std::cerr << "Failed to start server: " << e.what() << std::endl;
        shutdown();
        return 1;
    }

    while (true) {

        {
            std::unique_lock lock(state->mtx);
            state->cv.wait(lock, [&] {
                return state->phase == MatchPhase::Running || shouldStop;
            });
        }

        if (shouldStop) break;

        gameThread = std::thread([&] {
            Loop::run(udpServer, state);
        });

        gameThread.join();

        if (shouldStop) break;

        //tcpServer->notifyMatchEnded();

        tcpServer->resetLobbyStartTime();
        Loop::reset();
        tcpServer->resetLobby();
        //clientManager->resetAll();

        {
            std::lock_guard lock(state->mtx);
            state->phase = MatchPhase::Lobby;
        }

        std::cout << "Server reset. Waiting for new clients...\n";
    }

    std::cout << "Shutting down..." << std::endl;

    shutdown();

    std::cout << "Stopped. Goodbye :)" << std::endl;
    return 0;
}
