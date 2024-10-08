#pragma once
#include "MST.hpp"

// Concrete class for Prim's Minimum Spanning Tree algorithm
class PrimMST : public MST
{
public:
    std::vector<Edge> findMST(const Graph &graph) override;
};