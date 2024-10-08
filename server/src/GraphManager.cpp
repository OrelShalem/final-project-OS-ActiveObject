// This file contains the implementation of the GraphManager class, which manages a graph data structure.
// The GraphManager class provides safe access to the shared graph object,
// ensuring that all operations on the graph are thread-safe.

#include "GraphManager.hpp"
#include <sstream>
#include <iostream>

// Constructor: Initializes the GraphManager with an empty graph
GraphManager::GraphManager() : graph(std::make_shared<Graph>(0)) {}

// Destructor: Clears any remaining resources
GraphManager::~GraphManager()
{
    graph.reset();
}

// Adds a new vertex to the graph in a thread-safe manner
void GraphManager::addVertex()
{
    // This code adds a new vertex to the graph in a thread-safe manner
    std::lock_guard<std::mutex> lock(graphMutex); // Acquire a lock on the graph mutex
    graph->addVertex();                           // Call the addVertex method on the graph object
    // The lock is automatically released when it goes out of scope
}

// Adds a new edge to the graph in a thread-safe manner
void GraphManager::addEdge(int source, int destination, int weight)
{
    std::lock_guard<std::mutex> lock(graphMutex);
    graph->addEdge(source, destination, weight);
}

// Removes a vertex from the graph in a thread-safe manner
void GraphManager::removeVertex(int vertex)
{
    std::lock_guard<std::mutex> lock(graphMutex);
    graph->removeVertex(vertex);
}

// Removes an edge from the graph in a thread-safe manner
void GraphManager::removeEdge(int source, int destination)
{
    std::lock_guard<std::mutex> lock(graphMutex);
    graph->removeEdge(source, destination);
}

// Returns a shared pointer to the graph in a thread-safe manner
std::shared_ptr<Graph> GraphManager::getGraph() const
{
    std::lock_guard<std::mutex> lock(graphMutex);
    return graph;
}

bool GraphManager::changeWeight(int source, int destination, int newWeight)
{
    std::lock_guard<std::mutex> lock(graphMutex);
    return graph->changeWeight(source, destination, newWeight);
}

std::vector<Edge> GraphManager::getAdjacentEdges(int vertex) const
{
    std::lock_guard<std::mutex> lock(graphMutex);
    return graph->getAdjacentEdges(vertex);
}
int GraphManager::getVertices() const
{
    std::lock_guard<std::mutex> lock(graphMutex);
    return graph->getVertices();
}
int GraphManager::getEdges() const
{
    std::lock_guard<std::mutex> lock(graphMutex);
    return graph->getEdges();
}

// Returns a string representation of the graph for debugging purposes
std::string GraphManager::getGraphString() const
{
    std::lock_guard<std::mutex> lock(graphMutex);
    std::stringstream ss;
    ss << "Current graph:\n";
    ss << "Graph has " << graph->getVertices() << " vertices.\n";
    for (int i = 0; i < graph->getVertices(); ++i)
    {
        ss << "Vertex " << i << ":\n";
        const auto &edges = graph->getAdjacentEdges(i);
        ss << "This vertex has " << edges.size() << " edges.\n";
        for (const auto &edge : edges)
        {
            ss << "-> " << edge.destination << " (weight: " << edge.weight << ")\n";
        }
    }
    return ss.str();
}