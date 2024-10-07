#pragma once
#include "Pipeline.hpp"
#include "../../common/MSTFactory.hpp"
#include <string>
#include <atomic>
#include <vector>
#include <thread>
#include <mutex>

class Server
{
public:
    Server(int port);
    void start();
    void stop();
    bool isRunning() const { return running; }
    ~Server();

private:
    int port;
    std::atomic<bool> running;
    GraphManager graphManager;
    Pipeline pipeline;
    int serverSocket;
    std::thread acceptThread;
    std::vector<std::thread> clientThreads;
    std::vector<int> clientSockets;
    std::mutex clientThreadsMutex;
    std::mutex clientSocketsMutex;

    void handleClient(int clientSocket);
    void acceptClients();
    std::string getMSTString(const std::vector<Edge> &mst, const std::string &algorithm);
};