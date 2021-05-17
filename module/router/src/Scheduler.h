#pragma once

#include "database/src/Database.h"
#include "SingleNetRouter.h"
#include <Setting.h>

class Scheduler {
public:
    Scheduler(const vector<SingleNetRouter>& routersToExec, db::Setting& settingData, db::Database& db) : database(db), setting{settingData}, routers(routersToExec) {};
	db::Database& database;
	db::Setting& setting;
    vector<vector<int>>& schedule();

private:
    const vector<SingleNetRouter>& routers;
    vector<vector<int>> batches;

    // for conflict detect
    RTrees rtrees;
    virtual void initSet(vector<int> jobIdxes);
    virtual void updateSet(int jobIdx);
    virtual bool hasConflict(int jobIdx);
};

class PostScheduler {
public:
    PostScheduler(const vector<db::Net>& netsToExec, db::Database& db) : database(db), dbNets(netsToExec){};
    vector<vector<int>>& schedule();
	db::Database& database;

private:
    const vector<db::Net>& dbNets;
    vector<vector<int>> batches;

    // for conflict detect
    RTrees rtrees;
    boostBox getBoostBox(const db::GridPoint &gp);
    boostBox getBoostBox(const db::GridEdge &edge);
    vector<std::pair<boostBox, int>> getNetBoxes(const db::Net& dbNet);
    virtual void initSet(vector<int> jobIdxes);
    virtual void updateSet(int jobIdx);
    virtual bool hasConflict(int jobIdx);   
};
