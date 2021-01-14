#pragma once

#include "database/src/Database.h"
//todo
//#include "SingleNetRouter.h"

class Router {
public:
    void run();

private:
    int iter = 0;
	std::vector<float> _netsCost;
//    vector<db::RouteStatus> allNetStatus;
	db::Database Database;

	std::vector<int> getNetsToRoute();
    void ripup(const std::vector<int>& netsToRoute);
    void updateCost(const std::vector<int>& netsToRoute);
    void route(const std::vector<int>& netsToRoute);
    void finish();
    void unfinish();

    void printStat(bool major = false);
};
