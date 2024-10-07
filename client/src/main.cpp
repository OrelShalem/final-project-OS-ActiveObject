// This file contains the main client application for interacting with a graph server.

#include "Client.hpp"
#include <iostream>
#include <string>

// Function to print available commands and their descriptions
void printHelp()
{
    std::cout << "Available commands:\n"
              << "  add_vertex              - Add a new vertex to the graph\n"
              << "  add_edge <v1> <v2> <w>  - Add an edge between vertices v1 and v2 with weight w\n"
              << "  remove_vertex <v>       - Remove vertex v from the graph\n"
              << "  remove_edge <v1> <v2>   - Remove the edge between vertices v1 and v2\n"
              << "  calculate_mst <algo>    - Calculate the Minimum Spanning Tree using 'prim' or 'kruskal'\n"
              << "  metrics_mst             - Get the MST and its metrics\n"
              << "  help                    - Show this help message\n"
              << "  quit                    - Exit the program\n";
}

int main()
{
    // Create a Client object to connect to the server at localhost:9036
    Client client("127.0.0.1", 9036);

    try
    {
        // Attempt to connect to the server
        client.connect();
        std::cout << "Connected to server. Type 'help' for available commands.\n";

        // Main loop for user interaction
        while (true)
        {
            std::string request;
            std::cout << "\nEnter command: ";
            std::getline(std::cin, request);

            // Check for quit command to exit the program
            if (request == "quit")
            {
                break;
            }
            // Check for help command to display available commands
            else if (request == "help")
            {
                printHelp();
                continue;
            }

            // Send the user's request to the server
            client.sendRequest(request);
        }

        // Disconnect from the server before exiting
        client.disconnect();
    }
    catch (const std::exception &e)
    {
        // Handle any exceptions that occur during client operation
        std::cerr << "Client error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}