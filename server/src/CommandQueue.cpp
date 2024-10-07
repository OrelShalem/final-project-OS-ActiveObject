// This file implements a thread-safe CommandQueue class for managing commands in a concurrent environment.

#include <mutex>
#include <condition_variable>
#include <queue>

class CommandQueue
{
private:
    std::queue<std::string> commands; // Queue to store command strings
    std::mutex queue_mutex;           // Mutex for thread-safe access to the queue
    std::condition_variable cv;       // Condition variable for signaling queue state changes

public:
    // Adds a command to the queue
    void push(const std::string &cmd)
    {
        std::lock_guard<std::mutex> lock(queue_mutex); // Lock the mutex to ensure thread-safe access
        commands.push(cmd);                            // Add the command to the queue
        cv.notify_one();                               // Notify one waiting thread that a command is available
    }

    // Removes and returns a command from the queue
    std::string pop()
    {
        std::unique_lock<std::mutex> lock(queue_mutex); // Lock the mutex
        cv.wait(lock, [this]
                { return !commands.empty(); }); // Wait until the queue is not empty
        std::string cmd = commands.front();     // Get the first command
        commands.pop();                         // Remove the first command
        return cmd;                             // Return the command
    }
};