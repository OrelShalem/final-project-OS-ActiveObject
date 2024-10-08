#pragma once
#include <vector>
#include "Graph.hpp"

// Abstract class for Minimum Spanning Tree algorithms
class MST
{
public:
    // find the MST of the graph
    virtual std::vector<Edge> findMST(const Graph &graph) = 0;
    // destructor
    virtual ~MST() = default;
};