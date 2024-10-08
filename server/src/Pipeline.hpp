#pragma once
#include "ActiveObject.hpp"
#include "GraphManager.hpp"
#include "../../common/MSTMetrics.hpp"
#include <vector>
#include <memory>
#include <functional>
#include <string>

class Pipeline
{
public:
    Pipeline(GraphManager &graphManager);
    void start();
    void stop();
    ~Pipeline();
    void calculateMetrics(const Graph &graph, const std::vector<Edge> &mst, std::function<void(const std::string &)> responseCallback);

private:
    std::vector<std::unique_ptr<ActiveObject>> activeObjects;
    GraphManager &graphManager;
    // std::string getMetricsString(const MSTMetrics &metrics, const Graph &graph, const std::vector<Edge> &mst);
};