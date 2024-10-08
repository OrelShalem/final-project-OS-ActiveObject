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
            return;
        }
        running = false;
        condition.notify_one();
    }
    if (workerThread.joinable())
    {
        workerThread.join();
    }
}

// The main loop of the worker thread
void ActiveObject::run()
{
    while (true)
    {
        // Declare a function object to hold the task
        std::function<void()> task;
        {
            // Lock the queue mutex to safely access shared data
            std::unique_lock<std::mutex> lock(queueMutex);

            // Wait until there's a task in the queue or the thread is stopped
            condition.wait(lock, [this]
                           { return !taskQueue.empty() || !running; });

            // If the thread is stopped and there are no tasks, exit the loop
            if (!running && taskQueue.empty())
            {
                break; // Exit the loop if stopped and no tasks left
            }

            // If there's a task in the queue, move it to our local variable
            if (!taskQueue.empty())
            {
                task = std::move(taskQueue.front());
                taskQueue.pop();
            }
        } // The lock is released here

        // If we got a task, execute it
        if (task)
        {
            try
            {
                task(); // Execute the task
            }
            catch (const std::exception &e)
            {
                // Handle any standard exceptions
                std::cerr << "Exception in task execution: " << e.what() << std::endl;
            }
            catch (...)
            {
                // Handle any other types of exceptions
                std::cerr << "Unknown exception in task execution" << std::endl;
            }
        }
    }
}