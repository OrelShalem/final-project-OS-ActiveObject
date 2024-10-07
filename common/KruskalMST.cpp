#include "KruskalMST.hpp"
#include <algorithm>
#include <queue>
#include <stdexcept>
#include <limits>
#include <vector>
#include <functional>

using namespace std;

// This function implements Kruskal's algorithm to find the Minimum Spanning Tree (MST) of a graph
vector<Edge> KruskalMST::findMST(const Graph &graph)
{
    vector<Edge> mst;                      // Will store the edges of the MST
    vector<Edge> allEdges;                 // Will store all edges of the graph
    int numVertices = graph.getVertices(); // Get the number of vertices in the graph

    // Collect all edges from the graph
    for (int i = 0; i < numVertices; ++i)
    {
        vector<Edge> adjacentEdges = graph.getAdjacentEdges(i);
        allEdges.insert(allEdges.end(), adjacentEdges.begin(), adjacentEdges.end());
    }

    // Sort edges by weight in ascending order
    sort(allEdges.begin(), allEdges.end());

    // Initialize disjoint set data structure
    vector<int> parent(numVertices);
    for (int i = 0; i < numVertices; ++i)
    {
        parent[i] = i; // Each vertex is initially its own parent
    }

    // Find function for disjoint set (with path compression)
    function<int(int)> find = [&](int v)
    {
        if (parent[v] != v)
        {
            parent[v] = find(parent[v]); // Path compression
        }
        return parent[v];
    };

    // Union function for disjoint set
    auto unionSets = [&](int x, int y)
    {
        int rootX = find(x);
        int rootY = find(y);
        if (rootX != rootY)
        {
            parent[rootX] = rootY; // Union by rank can be added for optimization
        }
    };

    // Kruskal's algorithm main loop
    for (const Edge &edge : allEdges)
    {
        int sourceRoot = find(edge.source);
        int destRoot = find(edge.destination);

        // If the edge doesn't create a cycle, add it to the MST
        if (sourceRoot != destRoot)
        {
            mst.push_back(edge);
            unionSets(sourceRoot, destRoot);
        }

        // If we've added n-1 edges, we're done (n is the number of vertices)
        if (mst.size() == static_cast<size_t>(numVertices) - 1)
        {
            break;
        }
    }

    return mst; // Return the Minimum Spanning Tree
}