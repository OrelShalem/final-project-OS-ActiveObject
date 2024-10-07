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

    auto &sourceEdges = adjacencyList[source];
    auto &destEdges = adjacencyList[destination];

    auto sourceIt = std::remove_if(sourceEdges.begin(), sourceEdges.end(),
                                   [destination](const Edge &e)
                                   { return e.destination == destination; });
    auto destIt = std::remove_if(destEdges.begin(), destEdges.end(),
                                 [source](const Edge &e)
                                 { return e.destination == source; });

    bool removed = sourceIt != sourceEdges.end() || destIt != destEdges.end();

    sourceEdges.erase(sourceIt, sourceEdges.end());
    destEdges.erase(destIt, destEdges.end());

    return removed;
}

// Removes a vertex and all its incident edges from the graph
bool Graph::removeVertex(int vertex)
{
    if (adjacencyList.find(vertex) == adjacencyList.end())
    {
        return false;
    }

    adjacencyList.erase(vertex);

    for (auto &pair : adjacencyList)
    {
        auto &edges = pair.second;
        edges.erase(std::remove_if(edges.begin(), edges.end(),
                                   [vertex](const Edge &e)
                                   { return e.destination == vertex; }),
                    edges.end());
    }

    return true;
}

// Changes the weight of an edge between two vertices
bool Graph::changeWeight(int source, int destination, int newWeight)
{
    if (adjacencyList.find(source) == adjacencyList.end() || adjacencyList.find(destination) == adjacencyList.end())
    {
        return false;
    }

    auto &sourceEdges = adjacencyList[source];
    auto &destEdges = adjacencyList[destination];

    auto sourceIt = std::find_if(sourceEdges.begin(), sourceEdges.end(),
                                 [destination](const Edge &e)
                                 { return e.destination == destination; });
    auto destIt = std::find_if(destEdges.begin(), destEdges.end(),
                               [source](const Edge &e)
                               { return e.destination == source; });

    if (sourceIt == sourceEdges.end() || destIt == destEdges.end())
    {
        return false;
    }

    sourceIt->weight = newWeight;
    destIt->weight = newWeight;

    return true;
}

// Returns a vector of edges adjacent to a given vertex
std::vector<Edge> Graph::getAdjacentEdges(int vertex) const
{
    auto it = adjacencyList.find(vertex);
    if (it != adjacencyList.end())
    {
        return it->second;
    }
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