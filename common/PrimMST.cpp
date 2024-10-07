// This file implements Prim's algorithm for finding the Minimum Spanning Tree (MST) of a graph.

#include "PrimMST.hpp"
#include <queue>
#include <iostream>
#include <limits>

using namespace std;

vector<Edge> PrimMST::findMST(const Graph &graph)
{
    // Initialize the MST vector to store the edges of the minimum spanning tree
    vector<Edge> mst;
    int n = graph.getVertices();
    std::cout << "Prim's algorithm: Graph has " << n << " vertices" << std::endl;

    // Check if the graph is empty
    if (n == 0)
    {
        std::cout << "Graph is empty" << std::endl;
        return mst;
    }

    // Initialize data structures for Prim's algorithm
    vector<bool> visited(n, false);                 // Track visited vertices
    vector<int> key(n, numeric_limits<int>::max()); // Store minimum edge weights
    vector<int> parent(n, -1);                      // Store parent vertices in the MST

    // Priority queue to efficiently select the minimum weight edge
    priority_queue<pair<int, int>, vector<pair<int, int>>, greater<pair<int, int>>> pq;

    // Start with vertex 0
    int startVertex = 0;
    pq.push({0, startVertex});
    key[startVertex] = 0;

    // Main loop of Prim's algorithm
    while (!pq.empty())
    {
        int u = pq.top().second;
        pq.pop();

        // Skip if already visited
        if (visited[u])
            continue;

        visited[u] = true;

        // Add the edge to the MST if it's not the starting vertex
        if (parent[u] != -1)
        {
            mst.push_back({parent[u], u, key[u]});
            std::cout << "Adding edge to MST: (" << parent[u] << ", " << u << ", " << key[u] << ")" << std::endl;
        }

        // Explore neighbors of the current vertex
        for (const auto &neighbor : graph.getAdjacentEdges(u))
        {
            int v = neighbor.destination;
            int weight = neighbor.weight;

            std::cout << "Checking edge: (" << u << ", " << v << ", " << weight << ")" << std::endl;

            // Update the key if a shorter path is found
            if (!visited[v] && weight < key[v])
            {
                parent[v] = u;
                key[v] = weight;
                pq.push({key[v], v});
                std::cout << "Updated key for vertex " << v << " to " << weight << std::endl;
            }
        }
    }

    std::cout << "Prim's algorithm finished. MST has " << mst.size() << " edges" << std::endl;
    return mst;
}