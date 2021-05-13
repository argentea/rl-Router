#pragma once

#include "GridGraphBuilder.h"
#include "database/src/Database.h"
#include "router/src/LocalNet.h"
#include "router/src/RouterGraph.h"

class MazeRoute {
public:
    MazeRoute(LocalNet &localNetData, db::Setting& settingData, db::RrrIterSetting& rrrIterSettingData) : localNet(localNetData), graph(settingData, rrrIterSettingData) {}

    db::RouteStatus run();

private:
    LocalNet &localNet;
    RouterGraph graph;

    vector<db::CostT> vertexCostUBs;       // min cost upper bound for each vertex
    vector<db::CostT> vertexCostLBs;       // cost lower bound corresponding to the min-upper-bound solution for each vertex
    vector<std::shared_ptr<Solution>> pinSols;  // best solution for each pin

    db::RouteStatus route(int startPin);
    void getResult();
};
