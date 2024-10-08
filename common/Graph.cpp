// This file implements the Graph class, which represents an undirected weighted graph.

#include "Graph.hpp"
#include <algorithm>
#include <stdexcept>

// Constructor: Initializes the graph with a given number of vertices
Graph::Graph(int numVertices) : nextVertexId(numVertices)
{
    for (int i = 0; i < numVertices; ++i)
    {
        adjacencyList[i] = std::vector<Edge>();
    }
}

// Destructor: Clears the adjacency list
Graph::~Graph()
{
    adjacencyList.clear();
}

// Adds an edge between two vertices with a given weight
void Graph::addEdge(int source, int destination, int weight)
{
    if (adjacencyList.find(source) == adjacencyList.end() || adjacencyList.find(destination) == adjacencyList.end())
    {
        throw std::out_of_range("Vertex does not exist");
    }
    adjacencyList[source].push_back(Edge(source, destination, weight));
    adjacencyList[destination].push_back(Edge(destination, source, weight));
}

// Adds a new vertex to the graph and returns its ID
int Graph::addVertex()
{
    int newVertexId = nextVertexId++;
    adjacencyList[newVertexId] = std::vector<Edge>();
    return newVertexId;
}

// Removes an edge between two vertices if it exists
bool Graph::removeEdge(int source, int destination)
{
    if (adjacencyList.find(source) == adjacencyList.end() || adjacencyList.find(destination) == adjacencyList.end())
    {
        return false;
    }
    // Get references to the edge lists for both the source and destination vertices
    auto &sourceEdges = adjacencyList[source];
    auto &destEdges = adjacencyList[destination];

    // Use std::remove_if to find edges to remove from the source vertex
    // This moves matching edges to the end of the vector and returns an iterator to the first removed element
    auto sourceIt = std::remove_if(sourceEdges.begin(), sourceEdges.end(),
                                   [destination](const Edge &e)
                                   { return e.destination == destination; });

    // Similarly, find edges to remove from the destination vertex
    auto destIt = std::remove_if(destEdges.begin(), destEdges.end(),
                                 [source](const Edge &e)
                                 { return e.destination == source; });

    // Check if any edges were found to remove
    // If sourceIt or destIt are not at the end, it means at least one edge was found
    bool removed = sourceIt != sourceEdges.end() || destIt != destEdges.end();

    // Erase the removed edges from both vertex edge lists
    // This actually removes the elements from the end of the vector
    sourceEdges.erase(sourceIt, sourceEdges.end());
    destEdges.erase(destIt, destEdges.end());

    // Return whether any edges were removed
    return removed;
}

// Removes a vertex and all its incident edges from the graph
bool Graph::removeVertex(int vertex)
{
    if (adjacencyList.find(vertex) == adjacencyList.end())
    {
        return false;
    }

    // Remove the vertex from the adjacency list
    adjacencyList.erase(vertex);

    // Iterate through all remaining vertices in the adjacency list
    for (auto &pair : adjacencyList)
    {
        auto &edges = pair.second;
        // Remove any edges that have the deleted vertex as their destination
        edges.erase(std::remove_if(edges.begin(), edges.end(),
                                   [vertex](const Edge &e)
                                   { return e.destination == vertex; }),
                    edges.end());
    }

    // Return true to indicate successful removal
    return true;
}

// Changes the weight of an edge between two vertices
bool Graph::changeWeight(int source, int destination, int newWeight)
{
    if (adjacencyList.find(source) == adjacencyList.end() || adjacencyList.find(destination) == adjacencyList.end())
    {
        return false;
    }

    // Get references to the edge lists for both the source and destination vertices
    auto &sourceEdges = adjacencyList[source];
    auto &destEdges = adjacencyList[destination];

    // Find the edge from source to destination
    auto sourceIt = std::find_if(sourceEdges.begin(), sourceEdges.end(),
                                 [destination](const Edge &e)
                                 { return e.destination == destination; });

    // Find the edge from destination to source
    auto destIt = std::find_if(destEdges.begin(), destEdges.end(),
                               [source](const Edge &e)
                               { return e.destination == source; });

    // If either edge is not found, return false (edge doesn't exist)
    if (sourceIt == sourceEdges.end() || destIt == destEdges.end())
    {
        return false;
    }

    // Update the weight of both edges
    sourceIt->weight = newWeight;
    destIt->weight = newWeight;

    // Return true to indicate successful weight change
    return true;
}

// Returns a vector of edges adjacent to a given vertex
std::vector<Edge> Graph::getAdjacentEdges(int vertex) const
{
    // Search for the given vertex in the adjacency list
    auto it = adjacencyList.find(vertex);

    // If the vertex is found in the adjacency list
    if (it != adjacencyList.end())
    {
        // Return the vector of edges associated with this vertex
        return it->second;
    }

    // If the vertex is not found, return an empty vector of edges
    return std::vector<Edge>();
}

// Returns the number of vertices in the graph
int Graph::getVertices() const
{
    return adjacencyList.size();
}

// Returns the number of edges in the graph
int Graph::getEdges() const
{
    int count = 0;
    for (const auto &pair : adjacencyList)
    {
        count += pair.second.size();
    }
    return count / 2; // Each edge is counted twice
}