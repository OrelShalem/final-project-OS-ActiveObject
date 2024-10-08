#pragma once
#include <queue>
#include <mutex>
#include <condition_variable>
#include <functional>
#include <thread>

class ActiveObject
{
public:
    ActiveObject();
    ~ActiveObject();

    void enqueue(std::function<void()> task);
    void start();
    void stop();

private:
    std::queue<std::function<void()>> taskQueue;
    std::mutex queueMutex;
    std::condition_variable condition;
    std::thread workerThread;
    bool running;
    void run();
};