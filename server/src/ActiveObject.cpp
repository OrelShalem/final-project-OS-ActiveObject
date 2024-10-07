// This file implements the ActiveObject class, which provides a mechanism for asynchronous task execution.

#include "ActiveObject.hpp"
#include <iostream>

// Constructor: Initializes the ActiveObject with running set to false
ActiveObject::ActiveObject() : running(false) {}

// Destructor: Ensures that the ActiveObject is stopped before destruction
ActiveObject::~ActiveObject()
{
    stop();
}

// Enqueues a task for later execution
void ActiveObject::enqueue(std::function<void()> task)
{
    {
        std::lock_guard<std::mutex> lock(queueMutex);
        taskQueue.push(std::move(task));
    }
    condition.notify_one(); // Notify the worker thread that a new task is available
}

// Starts the ActiveObject's worker thread
void ActiveObject::start()
{
    {
        std::lock_guard<std::mutex> lock(queueMutex);
        if (running)
        {
            return; // Already running, do nothing
        }
        running = true;
    }
    workerThread = std::thread(&ActiveObject::run, this); // Start the worker thread
}

// Stops the ActiveObject's worker thread
void ActiveObject::stop()
{
    {
        std::lock_guard<std::mutex> lock(queueMutex);
        if (!running)
        {
            return; // Already stopped, do nothing
        }
        running = false;
    }
    condition.notify_one(); // Wake up the worker thread if it's waiting
    if (workerThread.joinable())
    {
        workerThread.join(); // Wait for the worker thread to finish
    }
}

// The main loop of the worker thread
void ActiveObject::run()
{
    while (true)
    {
        std::function<void()> task;
        {
            std::unique_lock<std::mutex> lock(queueMutex);
            condition.wait(lock, [this]
                           { return !taskQueue.empty() || !running; });
            if (!running && taskQueue.empty())
            {
                break; // Exit the loop if stopped and no tasks left
            }
            if (!taskQueue.empty())
            {
                task = std::move(taskQueue.front());
                taskQueue.pop();
            }
        }
        if (task)
        {
            try
            {
                task(); // Execute the task
            }
            catch (const std::exception &e)
            {
                std::cerr << "Exception in task execution: " << e.what() << std::endl;
            }
            catch (...)
            {
                std::cerr << "Unknown exception in task execution" << std::endl;
            }
        }
    }
}