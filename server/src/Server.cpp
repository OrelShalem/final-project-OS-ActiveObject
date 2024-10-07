#include "Server.hpp"
#include <iostream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <thread>
#include <algorithm>
#include <csignal>
#include <atomic>
#include <sstream>
#include <map>
#include <functional>
#include <cstring>
#include <mutex>

// External declarations for global variables
extern std::atomic<bool> shutdownRequested;
extern Server *serverPtr;
std::mutex runningMutex;
std::mutex acceptThreadMutex;
std::mutex graphMutex;
extern std::mutex coutMutex;

// External declaration for signal handler (defined in main.cpp)
extern void signalHandler(int signum);

// Constructor: Initialize the server with a given port
Server::Server(int p) : port(p), running(false), pipeline(graphManager), serverSocket(-1) {}

// Destructor: Ensure the server is stopped when the object is destroyed
Server::~Server()
{
    stop();
}

// Start the server
void Server::start()
{
    {
        std::lock_guard<std::mutex> lock(coutMutex);
        std::cout << "Server starting..." << std::endl;
    }

    // Create a socket
    serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSocket == -1)
    {
        std::lock_guard<std::mutex> lock(coutMutex);
        std::cerr << "Error creating socket" << std::endl;
        return;
    }

    // Set socket options to allow address reuse
    int opt = 1;
    if (setsockopt(serverSocket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0)
    {
        std::lock_guard<std::mutex> lock(coutMutex);
        std::cerr << "Error setting socket options" << std::endl;
        close(serverSocket);
        return;
    }

    // Prepare the server address structure
    sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = INADDR_ANY;
    serverAddr.sin_port = htons(port);

    // Bind the socket to the address
    if (bind(serverSocket, (struct sockaddr *)&serverAddr, sizeof(serverAddr)) < 0)
    {
        if (errno == EADDRINUSE)
        {
            // If the address is already in use, attempt to kill the process using it
            std::lock_guard<std::mutex> lock(coutMutex);
            std::cout << "Address already in use. Attempting to kill the process..." << std::endl;

            std::string killCommand = "sudo kill $(sudo lsof -t -i:" + std::to_string(port) + ")";
            int result = system(killCommand.c_str());

            if (result == 0)
            {
                std::cout << "Successfully killed the process. Retrying to bind..." << std::endl;
                if (bind(serverSocket, (struct sockaddr *)&serverAddr, sizeof(serverAddr)) == 0)
                {
                    std::cout << "Successfully bound to the address after killing the previous process." << std::endl;
                }
                else
                {
                    std::cerr << "Failed to bind even after killing the process. Error: " << strerror(errno) << std::endl;
                    close(serverSocket);
                    return;
                }
            }
            else
            {
                std::cerr << "Failed to kill the process. Error: " << strerror(errno) << std::endl;
                close(serverSocket);
                return;
            }
        }
        else
        {
            std::lock_guard<std::mutex> lock(coutMutex);
            std::cerr << "Error binding socket: " << strerror(errno) << std::endl;
            close(serverSocket);
            return;
        }
    }

    // Start listening for incoming connections
    if (listen(serverSocket, 3) < 0)
    {
        std::lock_guard<std::mutex> lock(coutMutex);
        std::cerr << "Error listening on socket" << std::endl;
        close(serverSocket);
        return;
    }

    {
        std::lock_guard<std::mutex> lock(coutMutex);
        std::cout << "Server listening on port " << port << std::endl;
    }

    // Set the running flag to true
    {
        std::lock_guard<std::mutex> lock(runningMutex);
        running = true;
    }

    // Start the pipeline
    pipeline.start();

    // Start the thread that accepts client connections
    {
        std::lock_guard<std::mutex> lock(acceptThreadMutex);
        acceptThread = std::thread(&Server::acceptClients, this);
    }

    {
        std::lock_guard<std::mutex> lock(coutMutex);
        std::cout << "Server started successfully" << std::endl;
    }
}

// Stop the server
void Server::stop()
{
    {
        std::lock_guard<std::mutex> lock(runningMutex);
        if (!running)
        {
            return; // Server is already stopped
        }
        running = false;
    }

    {
        std::lock_guard<std::mutex> lock(coutMutex);
        std::cout << "Stopping server..." << std::endl;
    }

    // Close the server socket
    if (serverSocket != -1)
    {
        shutdown(serverSocket, SHUT_RDWR);
        close(serverSocket);
        serverSocket = -1;
    }

    // Stop accepting new clients
    if (acceptThread.joinable())
    {
        acceptThread.join();
    }

    // Close all client sockets and clear the vector
    {
        std::lock_guard<std::mutex> lock(clientSocketsMutex);
        for (int clientSocket : clientSockets)
        {
            shutdown(clientSocket, SHUT_RDWR);
            close(clientSocket);
        }
        clientSockets.clear();
    }

    // Wait for all client threads to finish
    {
        std::lock_guard<std::mutex> lock(clientThreadsMutex);
        for (auto &thread : clientThreads)
        {
            if (thread.joinable())
            {
                thread.join();
            }
        }
        clientThreads.clear();
    }

    // Stop the pipeline
    pipeline.stop();

    {
        std::lock_guard<std::mutex> lock(coutMutex);
        std::cout << "Server stopped" << std::endl;
    }
}

void Server::handleClient(int clientSocket)
{
    {
        std::lock_guard<std::mutex> lock(coutMutex);
        std::cout << "Handling client connection..." << std::endl;
    }

    char buffer[1024] = {0};
    while (running.load(std::memory_order_acquire))
    {
        int valread = read(clientSocket, buffer, 1024);
        if (valread <= 0)
        {
            std::lock_guard<std::mutex> lock(coutMutex);
            std::cout << "Client disconnected or error reading" << std::endl;
            break;
        }

        std::string message(buffer);
        {
            std::lock_guard<std::mutex> lock(coutMutex);
            std::cout << "Received message from client: " << message << std::endl;
        }

        std::istringstream iss(message);
        std::string command;
        iss >> command;

        auto sendResponse = [clientSocket, this](const std::string &response)
        {
            {
                std::lock_guard<std::mutex> lock(coutMutex);
                std::cout << "Sending response: " << response << std::endl;
            }
            send(clientSocket, response.c_str(), response.length(), 0);
        };

        {
            std::lock_guard<std::mutex> lock(coutMutex);
            std::cout << "Processing command: " << command << std::endl;
        }

        if (command == "calculate_mst")
        {
            std::string algorithm;
            if (iss >> algorithm)
            {
                {
                    std::lock_guard<std::mutex> lock(coutMutex);
                    std::cout << "MST algorithm: " << algorithm << std::endl;
                }
                if (algorithm == "prim" || algorithm == "kruskal")
                {
                    std::shared_ptr<Graph> graph;
                    {
                        std::lock_guard<std::mutex> lock(graphMutex);
                        graph = graphManager.getGraph();
                    }
                    {
                        std::lock_guard<std::mutex> lock(coutMutex);
                        std::cout << "Graph vertices: " << graph->getVertices() << std::endl;
                    }

                    auto mstCalculator = MSTFactory::createMST(algorithm);
                    if (!mstCalculator)
                    {
                        sendResponse("Error: Failed to create MST calculator");
                        continue;
                    }

                    auto mst = mstCalculator->findMST(*graph);
                    {
                        std::lock_guard<std::mutex> lock(coutMutex);
                        std::cout << "MST edges: " << mst.size() << std::endl;
                    }

                    std::string mstStr = getMSTString(mst, algorithm);
                    sendResponse("Minimum Spanning Tree:\n" + mstStr);

                    // Calculate metrics using the pipeline
                    // MARK: Using pipeline here
                    pipeline.calculateMetrics(*graph, mst, sendResponse);
                }
                else
                {
                    sendResponse("Invalid algorithm. Use 'prim' or 'kruskal'.");
                }
            }
            else
            {
                sendResponse("Please specify the algorithm: calculate_mst <prim|kruskal>");
            }
        }
        else if (command == "add_vertex")
        {
            {
                std::lock_guard<std::mutex> lock(coutMutex);
                std::cout << "Processing add_vertex command" << std::endl;
            }
            {
                std::lock_guard<std::mutex> lock(graphMutex);
                graphManager.addVertex();
            }
            sendResponse("Vertex added successfully.\n" + graphManager.getGraphString());
        }
        else if (command == "add_edge")
        {
            int v1, v2, weight;
            if (iss >> v1 >> v2 >> weight)
            {
                {
                    std::lock_guard<std::mutex> lock(graphMutex);
                    graphManager.addEdge(v1, v2, weight);
                }
                sendResponse("Edge added successfully.\n" + graphManager.getGraphString());
            }
            else
            {
                sendResponse("Invalid edge format. Use: add_edge <v1> <v2> <weight>");
            }
        }
        else if (command == "remove_vertex")
        {
            int v;
            if (iss >> v)
            {
                {
                    std::lock_guard<std::mutex> lock(graphMutex);
                    graphManager.removeVertex(v);
                }
                sendResponse("Vertex removed successfully.\n" + graphManager.getGraphString());
            }
            else
            {
                sendResponse("Invalid vertex format. Use: remove_vertex <v>");
            }
        }
        else if (command == "remove_edge")
        {
            int v1, v2;
            if (iss >> v1 >> v2)
            {
                {
                    std::lock_guard<std::mutex> lock(graphMutex);
                    graphManager.removeEdge(v1, v2);
                }
                sendResponse("Edge removed successfully.\n" + graphManager.getGraphString());
            }
            else
            {
                sendResponse("Invalid edge format. Use: remove_edge <v1> <v2>");
            }
        }
        else if (command == "metrics_mst")
        {
            std::shared_ptr<Graph> graph;
            {
                std::lock_guard<std::mutex> lock(graphMutex);
                graph = graphManager.getGraph();
            }
            if (graph->getVertices() == 0)
            {
                sendResponse("Error: Graph is empty. Add vertices and edges before calculating MST metrics.");
            }
            else
            {
                std::string algorithm = (command == "calculate_mst" && iss >> algorithm) ? algorithm : "kruskal";
                if (algorithm != "prim" && algorithm != "kruskal")
                {
                    sendResponse("Invalid algorithm. Use 'prim' or 'kruskal'.");
                    continue;
                }

                {
                    std::lock_guard<std::mutex> lock(coutMutex);
                    std::cout << "Calculating MST using " << algorithm << " algorithm" << std::endl;
                    std::cout << "Graph vertices: " << graph->getVertices() << std::endl;
                }

                auto mstCalculator = MSTFactory::createMST(algorithm);
                if (!mstCalculator)
                {
                    sendResponse("Error: Failed to create MST calculator");
                    continue;
                }

                auto mst = mstCalculator->findMST(*graph);
                {
                    std::lock_guard<std::mutex> lock(coutMutex);
                    std::cout << "MST edges: " << mst.size() << std::endl;
                }

                std::string mstStr = getMSTString(mst, algorithm);
                sendResponse("Minimum Spanning Tree:\n" + mstStr);

                // Calculate metrics using the pipeline
                try
                {
                    pipeline.calculateMetrics(*graph, mst, [this, &sendResponse](const std::string &metricsStr)
                                              { sendResponse("MST Metrics:\n" + metricsStr); });
                }
                catch (const std::exception &e)
                {
                    std::lock_guard<std::mutex> lock(coutMutex);
                    std::cerr << "Error calculating metrics: " << e.what() << std::endl;
                    sendResponse("Error calculating metrics: " + std::string(e.what()));
                }
                catch (...)
                {
                    std::lock_guard<std::mutex> lock(coutMutex);
                    std::cerr << "Unknown error occurred while calculating metrics" << std::endl;
                    sendResponse("Unknown error occurred while calculating metrics");
                }
            }
        }
        else
        {
            sendResponse("Unknown command: " + command);
        }

        // Clear the buffer for the next message
        std::memset(buffer, 0, sizeof(buffer));
    }

    close(clientSocket);
    {
        std::lock_guard<std::mutex> lock(coutMutex);
        std::cout << "Client disconnected" << std::endl;
    }

    // Remove the client socket from the vector
    {
        std::lock_guard<std::mutex> lock(clientSocketsMutex);
        clientSockets.erase(std::remove(clientSockets.begin(), clientSockets.end(), clientSocket), clientSockets.end());
    }
}

// This function converts the Minimum Spanning Tree (MST) into a string representation
std::string Server::getMSTString(const std::vector<Edge> &mst, const std::string &algorithm)
{
    std::stringstream ss;
    // Add header indicating which algorithm was used
    ss << "MST created using " << (algorithm == "prim" ? "Prim's" : "Kruskal's") << " algorithm.\n";

    // Check if the MST is empty
    if (mst.empty())
    {
        ss << "The MST is empty. The graph might be disconnected or have no edges.\n";
        return ss.str();
    }

    // Create an adjacency list representation of the tree
    std::map<int, std::vector<std::pair<int, int>>> tree;
    for (const auto &edge : mst)
    {
        tree[edge.source].emplace_back(edge.destination, edge.weight);
        tree[edge.destination].emplace_back(edge.source, edge.weight);
    }

    // Recursive lambda function to print the tree structure
    std::function<void(int, int, std::string, std::string, bool)> printNode =
        [&](int node, int parent, std::string prefix, std::string childPrefix, bool isLast)
    {
        ss << prefix;
        if (parent != -1)
        {
            ss << (isLast ? "└─ " : "├─ ");
            auto it = std::find_if(tree[node].begin(), tree[node].end(),
                                   [parent](const auto &p)
                                   { return p.first == parent; });
            if (it != tree[node].end())
            {
                ss << "Node " << node << " [weight: " << it->second << "]";
            }
        }
        else
        {
            ss << "Node " << node;
        }
        ss << "\n";

        // Recursively print child nodes
        for (size_t i = 0; i < tree[node].size(); ++i)
        {
            const auto &[child, weight] = tree[node][i];
            if (child != parent)
            {
                bool isLastChild = (i == tree[node].size() - 1) ||
                                   (i == tree[node].size() - 2 && tree[node][i + 1].first == parent);
                printNode(child, node,
                          childPrefix + (isLast ? "   " : "│  "),
                          childPrefix + (isLast ? "   " : "│  "),
                          isLastChild);
            }
        }
    };

    // Start printing from the first node in the MST
    if (!mst.empty())
    {
        printNode(mst[0].source, -1, "", "", true);
    }
    return ss.str();
}

// This function handles accepting new client connections
void Server::acceptClients()
{
    while (running.load(std::memory_order_acquire))
    {
        sockaddr_in clientAddr;
        socklen_t clientAddrLen = sizeof(clientAddr);
        int clientSocket = accept(serverSocket, (struct sockaddr *)&clientAddr, &clientAddrLen);

        if (clientSocket < 0)
        {
            if (running.load(std::memory_order_acquire))
            {
                std::lock_guard<std::mutex> lock(coutMutex);
                std::cerr << "Error accepting client connection" << std::endl;
            }
            continue;
        }

        {
            std::lock_guard<std::mutex> lock(coutMutex);
            std::cout << "New client connected" << std::endl;
        }

        {
            std::lock_guard<std::mutex> socketLock(clientSocketsMutex);
            std::lock_guard<std::mutex> threadLock(clientThreadsMutex);

            // Add the new client socket to the list
            clientSockets.push_back(clientSocket);
            // Start a new thread to handle this client
            clientThreads.emplace_back(&Server::handleClient, this, clientSocket);
        }
    }
}