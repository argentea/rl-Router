#pragma once

//#include "GridGraphBuilder.h"
#include "database/src/Database.h"
#include "router/src/LocalNet.h"
#include "router/src/RouterGraph.h"

class Solution {
public:
    db::CostT cost;
//    DBU len;           // length on current track
    db::CostT costUB;  // cost upper bound
    int vertex;
    std::shared_ptr<Solution> prev;

    Solution(db::CostT c, db::CostT ub, int v, const std::shared_ptr<Solution> &p)
        : cost(c), costUB(ub), vertex(v), prev(p) {}

    friend ostream &operator<<(ostream &os, const Solution &sol);
};

class MazeRoute {
public:
    MazeRoute(LocalNet &localNetData) : localNet(localNetData) {}

    bool run();

private:
    LocalNet &localNet;
    RouterGraph graph;

    vector<db::CostT> vertexCostUBs;       // min cost upper bound for each vertex
    // vector<db::CostT> vertexCostLBs;       // cost lower bound corresponding to the min-upper-bound solution for each vertex
    vector<std::shared_ptr<Solution>> pinSols;  // best solution for each pin

    bool route(int startPin);
    void getResult();
};
