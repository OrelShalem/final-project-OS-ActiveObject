// This code is the main entry point for a server application.

// Include necessary headers
#include "Server.hpp"
#include <iostream>
#include <csignal>
#include <atomic>
#include <thread>
#include <chrono>
#include <mutex>

// Global variables
std::atomic<bool> shutdownRequested(false); // Flag to indicate if shutdown is requested
Server *serverPtr = nullptr;                // Pointer to the Server instance
std::mutex coutMutex;                       // Mutex for thread-safe console output

// Signal handler function
void signalHandler(int signum)
{
    {
        std::lock_guard<std::mutex> lock(coutMutex);
        std::cout << "Interrupt signal (" << signum << ") received. Stopping server..." << std::endl;
    }
    shutdownRequested.store(true); // Set the shutdown flag
    if (serverPtr && serverPtr->isRunning())
    {
        serverPtr->stop(); // Stop the server if it's running
    }
}

// Main function
int main()
{
    try
    {
        Server server(9036); // Create a Server instance on port 9036
        serverPtr = &server; // Set the global server pointer

        // Register signal handlers for SIGINT and SIGTERM
        std::signal(SIGINT, signalHandler);
        std::signal(SIGTERM, signalHandler);

        {
            std::lock_guard<std::mutex> lock(coutMutex);
            std::cout << "Starting server on port 9036..." << std::endl;
        }
        server.start(); // Start the server

        // Main loop: keep running until shutdown is requested or server stops
        while (!shutdownRequested.load() && server.isRunning())
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }

        // Stop the server if it's still running
        if (server.isRunning())
        {
            server.stop();
        }

        serverPtr = nullptr; // Reset the global server pointer
    }
    catch (const std::exception &e)
    {
        // Handle standard exceptions
        std::lock_guard<std::mutex> lock(coutMutex);
        std::cerr << "Server error: " << e.what() << std::endl;
        return 1;
    }
    catch (...)
    {
        // Handle unknown exceptions
        std::lock_guard<std::mutex> lock(coutMutex);
        std::cerr << "Unknown error occurred" << std::endl;
        return 1;
    }

    // Final shutdown message
    {
        std::lock_guard<std::mutex> lock(coutMutex);
        std::cout << "Server shutdown complete." << std::endl;
    }
    return 0;
}