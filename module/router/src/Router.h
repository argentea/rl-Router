#pragma once

#include "../../database/src/Database.h"
#include "SingleNetRouter.h"

class Router {
public:
    void run();

private:
    int iter = 0;
    vector<float> _netsCost;
    vector<db::RouteStatus> allNetStatus;

    vector<int> getNetsToRoute();
    void ripup(const vector<int>& netsToRoute);
    void updateCost(const vector<int>& netsToRoute);
    void route(const vector<int>& netsToRoute);
    void finish();
    void unfinish();

    void printStat(bool major = false);
};
