// This file implements the MSTFactory class, which creates MST (Minimum Spanning Tree) objects based on the specified algorithm.

#include "MSTFactory.hpp"
#include "PrimMST.hpp"
#include "KruskalMST.hpp"
#include <stdexcept>
#include <iostream>
#include <algorithm>

using namespace std;

// Factory method to create an MST object based on the specified algorithm
std::unique_ptr<MST> MSTFactory::createMST(const std::string &algorithm)
{
    // Log the algorithm being used
    std::cout << "Creating MST calculator for algorithm: " << algorithm << std::endl;

    // Convert the algorithm name to lowercase for case-insensitive comparison
    std::string lowerAlgorithm = algorithm;
    std::transform(lowerAlgorithm.begin(), lowerAlgorithm.end(), lowerAlgorithm.begin(), ::tolower);

    // Create and return the appropriate MST object based on the algorithm
    if (lowerAlgorithm == "prim")
    {
        return std::make_unique<PrimMST>(); // Create a PrimMST object
    }
    else if (lowerAlgorithm == "kruskal")
    {
        return std::make_unique<KruskalMST>(); // Create a KruskalMST object
    }
    else
    {
        // If the algorithm is unknown, log an error and throw an exception
        std::cerr << "Unknown MST algorithm: " << algorithm << std::endl;
        throw std::invalid_argument("Unknown MST algorithm: " + algorithm);
    }
}