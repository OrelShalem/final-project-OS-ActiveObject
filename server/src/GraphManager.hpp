#pragma once
#include "../../common/Graph.hpp"
#include <mutex>
#include <string>
#include <memory>

class GraphManager
{
public:
    GraphManager();

    void addVertex();
    void addEdge(int source, int destination, int weight);
    void removeVertex(int vertex);
    void removeEdge(int source, int destination);
    std::shared_ptr<Graph> getGraph() const;
    std::string getGraphString() const;
    bool changeWeight(int source, int destination, int newWeight);
    std::vector<Edge> getAdjacentEdges(int vertex) const;
    int getVertices() const;
    int getEdges() const;
    ~GraphManager();

private:
    std::shared_ptr<Graph> graph;
    mutable std::mutex graphMutex;
};