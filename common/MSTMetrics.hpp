#pragma once
#include "Graph.hpp"
#include <vector>
#include <limits>

class MSTMetrics
{
public:
    int getTotalWeight(const std::vector<Edge> &mst) const;
    int getLongestDistance(const Graph &graph, const std::vector<Edge> &mst) const;
    int getShortestDistance(const std::vector<Edge> &mst) const;
    double getAverageDistance(const Graph &graph, const std::vector<Edge> &mst) const;
};
