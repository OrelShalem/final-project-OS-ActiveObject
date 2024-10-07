// This file implements the MSTMetrics class, which provides methods to calculate various metrics
// for Minimum Spanning Trees (MSTs) in graphs.

#include "MSTMetrics.hpp"
#include <limits>
#include <algorithm>
#include <numeric>
#include <vector>
#include <queue>
#include <unordered_set>
#include <iostream>
#include <cassert>

using namespace std;

// Calculates the total weight of the MST
int MSTMetrics::getTotalWeight(const std::vector<Edge> &mst) const
{
    int total = 0;
    for (const auto &edge : mst)
    {
        if (edge.weight > 0)
        {
            total += edge.weight;
        }
    }
    return total;
}

// Finds the longest distance between any two vertices in the MST
int MSTMetrics::getLongestDistance(const Graph &graph, const std::vector<Edge> &mst) const
{
    // Check for empty MST or graph
    if (mst.empty() || graph.getVertices() == 0)
    {
        std::cout << "Empty MST or graph" << std::endl;
        return 0;
    }

    int numVertices = graph.getVertices();
    // Initialize distance matrix
    std::vector<std::vector<int>> dist(numVertices, std::vector<int>(numVertices, std::numeric_limits<int>::max()));

    // Populate distance matrix with MST edges
    for (const auto &edge : mst)
    {
        // Validate edge vertices
        if (edge.source < 0 || edge.source >= numVertices ||
            edge.destination < 0 || edge.destination >= numVertices)
        {
            std::cout << "Invalid edge: (" << edge.source << ", " << edge.destination << ", " << edge.weight << ")" << std::endl;
            continue;
        }
        dist[edge.source][edge.destination] = edge.weight;
        dist[edge.destination][edge.source] = edge.weight;
    }

    // Apply Floyd-Warshall algorithm for all-pairs shortest paths
    for (int k = 0; k < numVertices; ++k)
    {
        for (int i = 0; i < numVertices; ++i)
        {
            for (int j = 0; j < numVertices; ++j)
            {
                if (dist[i][k] != std::numeric_limits<int>::max() &&
                    dist[k][j] != std::numeric_limits<int>::max() &&
                    dist[i][k] + dist[k][j] < dist[i][j])
                {
                    dist[i][j] = dist[i][k] + dist[k][j];
                }
            }
        }
    }

    // Find the maximum distance
    int maxDist = 0;
    for (int i = 0; i < numVertices; ++i)
    {
        for (int j = i + 1; j < numVertices; ++j)
        {
            if (dist[i][j] != std::numeric_limits<int>::max())
            {
                maxDist = std::max(maxDist, dist[i][j]);
            }
        }
    }

    return maxDist;
}

// Calculates the average distance between all pairs of vertices in the MST
double MSTMetrics::getAverageDistance(const Graph &graph, const vector<Edge> &mst) const
{
    if (mst.empty())
    {
        return 0.0;
    }

    int numVertices = graph.getVertices();
    // Initialize distance matrix
    vector<vector<int>> dist(numVertices, vector<int>(numVertices, numeric_limits<int>::max()));

    // Populate distance matrix with MST edges
    for (const auto &edge : mst)
    {
        dist[edge.source][edge.destination] = edge.weight;
        dist[edge.destination][edge.source] = edge.weight;
    }

    // Apply Floyd-Warshall algorithm for all-pairs shortest paths
    for (int k = 0; k < numVertices; ++k)
    {
        for (int i = 0; i < numVertices; ++i)
        {
            for (int j = 0; j < numVertices; ++j)
            {
                if (dist[i][k] != numeric_limits<int>::max() &&
                    dist[k][j] != numeric_limits<int>::max() &&
                    dist[i][k] + dist[k][j] < dist[i][j])
                {
                    dist[i][j] = dist[i][k] + dist[k][j];
                }
            }
        }
    }

    // Calculate the sum of all distances and count valid pairs
    double totalDistance = 0.0;
    int validPairs = 0;
    for (int i = 0; i < numVertices; ++i)
    {
        for (int j = i + 1; j < numVertices; ++j)
        {
            if (dist[i][j] != numeric_limits<int>::max())
            {
                totalDistance += dist[i][j];
                validPairs++;
            }
        }
    }

    // Calculate and return the average distance
    return (validPairs > 0) ? (totalDistance / validPairs) : 0.0;
}

// Finds the shortest distance between any two vertices in the MST
int MSTMetrics::getShortestDistance(const vector<Edge> &mst) const
{
    if (mst.empty())
        return 0;

    size_t size = mst.size();
    // Initialize distance matrix
    vector<vector<int>> dist(size + 1, vector<int>(size + 1, numeric_limits<int>::max()));

    // Populate distance matrix with MST edges
    for (const auto &edge : mst)
    {
        dist[edge.source][edge.destination] = edge.weight;
        dist[edge.destination][edge.source] = edge.weight;
    }

    // Apply Floyd-Warshall algorithm for all-pairs shortest paths
    for (size_t k = 0; k <= size; ++k)
    {
        for (size_t i = 0; i <= size; ++i)
        {
            for (size_t j = 0; j <= size; ++j)
            {
                if (dist[i][k] != numeric_limits<int>::max() &&
                    dist[k][j] != numeric_limits<int>::max() &&
                    dist[i][k] + dist[k][j] < dist[i][j])
                {
                    dist[i][j] = dist[i][k] + dist[k][j];
                }
            }
        }
    }

    // Find the shortest distance between any two vertices
    int shortestDist = numeric_limits<int>::max();
    for (size_t i = 0; i <= size; ++i)
    {
        for (size_t j = i + 1; j <= size; ++j)
        {
            if (dist[i][j] != numeric_limits<int>::max())
            {
                shortestDist = min(shortestDist, dist[i][j]);
            }
        }
    }

    return shortestDist;
}