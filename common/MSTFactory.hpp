#pragma once
#include <memory>
#include <string>
#include "MST.hpp"
#include "PrimMST.hpp"
#include "KruskalMST.hpp"

using namespace std;
// Factory class for creating MST objects
class MSTFactory
{
public:
    // create the MST of the graph
    static unique_ptr<MST> createMST(const string &algorithm);
};