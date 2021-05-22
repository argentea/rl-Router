#pragma once

#include "RouteGuideGraph.h"
#include "database/src/Database.h"

class LocalNet : public RouteGuideGraph, public db::NetBase {
public:
    LocalNet(const db::Net& databaseNet, db::Setting& settingData, db::RrrIterSetting& rrrIterSettingData, db::Database& DB) :RouteGuideGraph(settingData, rrrIterSettingData, DB), db::NetBase(databaseNet), dbNet(databaseNet) {}

    const db::Net& dbNet;

    // note: routeGuides & pinAccessBoxes (inherited from db::NetBase) may be different from db::Net
    // (pinIdx, accessBoxIdx) -> (Grid)BoxOnLayer
    vector<vector<db::GridBoxOnLayer>> gridPinAccessBoxes;
    // guideIdx -> dbGuideIdxes
    vector<vector<int>> dbRouteGuideIdxes;

    int estimatedNumOfVertices = 0;

    void initGridBoxes(db::Database& database);
    void initNumOfVertices(db::Database& database);
    bool checkPin(db::Database& database) const;

    int getViaPenalty(int guideIdx1, int trackIdx1, int cpIdx1, int guideIdx2, int trackIdx2, int cpIdx2) const;
    double getWireSegmentPenalty(db::Database& database, int guideIdx, int trackIdx, int cpIdx1, int cpIdx2) const;
    int getCrossPointPenalty(int guideIdx, int trackIdx, int cpIdx) const;

    void print() const;  // db::NetBase::print() + RouteGuideGraph::print()

private:
    void getRouteGuideMapping();
};
