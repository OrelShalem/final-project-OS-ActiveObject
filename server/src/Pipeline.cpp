#include "Pipeline.hpp"
#include "../../common/MSTMetrics.hpp"
#include <sstream>
#include <iostream>
#include <mutex>

// This line declares an external mutex named 'coutMutex'
// It's used to synchronize access to std::cout across multiple threads
// By declaring it as extern, we're indicating that this mutex is defined elsewhere
// (likely in another source file) and we're just referencing it here
extern std::mutex coutMutex;

// The Pipeline class manages the processing of graph-related tasks using Active Objects
Pipeline::Pipeline(GraphManager &gm) : graphManager(gm)
{
    // Initialize the pipeline with multiple Active Objects
    for (int i = 0; i < 5; ++i)
    {
        activeObjects.push_back(std::make_unique<ActiveObject>());
    }
}

Pipeline::~Pipeline()
{
    stop();
}

// Start all Active Objects in the pipeline
void Pipeline::start()
{
    for (auto &ao : activeObjects)
    {
        ao->start();
    }
}

// Stop all Active Objects in the pipeline
void Pipeline::stop()
{
    for (auto &ao : activeObjects)
    {
        ao->stop();
    }
}

// Calculate metrics for a given graph and its Minimum Spanning Tree (MST)
void Pipeline::calculateMetrics(const Graph &graph, const std::vector<Edge> &mst, std::function<void(const std::string &)> responseCallback)
{
    activeObjects[0]->enqueue([this, graph, mst, responseCallback]()
                              {
        try {
            // Log initial information about the graph and MST
            {
                std::lock_guard<std::mutex> lock(coutMutex);
                std::cout << "Calculating metrics" << std::endl;
                std::cout << "Graph vertices: " << graph.getVertices() << std::endl;
                std::cout << "MST edges: " << mst.size() << std::endl;
            }

            // Check if the MST is valid
            if (mst.empty() || graph.getVertices() < 2)
            {
                std::lock_guard<std::mutex> lock(coutMutex);
                std::cerr << "Error: MST is empty or graph has less than 2 vertices" << std::endl;
                responseCallback("Error: Cannot calculate metrics. MST is empty or graph has less than 2 vertices.");
                return;
            }

            // Log MST edges for debugging
            {
                std::lock_guard<std::mutex> lock(coutMutex);
                std::cout << "MST edges:" << std::endl;
                for (const auto &edge : mst)
                {
                    std::cout << "(" << edge.source << ", " << edge.destination << ", " << edge.weight << ")" << std::endl;
                }
            }

            // Use the second Active Object to calculate total weight and longest distance
            activeObjects[1]->enqueue([this, graph, mst, responseCallback]()
            {
                MSTMetrics metrics;
                int totalWeight = metrics.getTotalWeight(mst);
                
                
                // Use the third Active Object to calculate shortest and average distance
                activeObjects[2]->enqueue([this, graph, mst, totalWeight, responseCallback]()
                {
                    MSTMetrics metrics;
                    int longestDistance = metrics.getLongestDistance(graph, mst);
                    
                    activeObjects[3]->enqueue([this, graph, mst , totalWeight, longestDistance, responseCallback]()
                    {   
                        MSTMetrics metrics;
                        int shortestDistance = metrics.getShortestDistance(mst);
                        
                        activeObjects[4]->enqueue([this, graph, mst , totalWeight, longestDistance, shortestDistance, responseCallback]()
                        {   
                            MSTMetrics metrics;
                            double averageDistance = metrics.getAverageDistance(graph, mst);
                            
                            // Prepare the response string with calculated metrics
                            std::stringstream ss;
                            ss << "MST Metrics:\n";
                            ss << "Total Weight: " << totalWeight << "\n";
                            ss << "Longest Distance: " << longestDistance << "\n";
                            ss << "Shortest Distance: " << shortestDistance << "\n";
                            ss << "Average Distance: " << averageDistance << "\n";

                            // Log and send the response
                            {
                                std::lock_guard<std::mutex> lock(coutMutex);
                                std::cout << "Sending response" << std::endl;
                            }
                            responseCallback(ss.str());
                    
                        });
                    });
                });
            });
        } catch (const std::exception& e) {
            std::lock_guard<std::mutex> lock(coutMutex);
            std::cerr << "Error calculating metrics: " << e.what() << std::endl;
            responseCallback("Error calculating metrics: " + std::string(e.what()));
        } catch (...) {
            std::lock_guard<std::mutex> lock(coutMutex);
            std::cerr << "Unknown error occurred while calculating metrics" << std::endl;
            responseCallback("Unknown error occurred while calculating metrics");
        } });
}

// // Helper function to format metrics as a string
// std::string Pipeline::getMetricsString(const MSTMetrics &metrics, const Graph &graph, const std::vector<Edge> &mst)
// {
//     std::stringstream ss;
//     ss << "Total Weight: " << metrics.getTotalWeight(mst) << "\n";
//     ss << "Longest Distance: " << metrics.getLongestDistance(graph, mst) << "\n";
//     ss << "Shortest Distance: " << metrics.getShortestDistance(mst) << "\n";
//     ss << "Average Distance: " << metrics.getAverageDistance(graph, mst) << "\n";
//     return ss.str();
// }