#pragma once

#include "../../database/src/Database.h"
#include "SingleNetRouter.h"
#include <Setting.h>

class Router {
public:
    void run();
	db::Setting& setting;
	db::RrrIterSetting& rrrIterSetting;

private:
	Router(db::Database& databaseData, db::Setting& settingData, db::RrrIterSetting& rrrIterSettingData): rrrIterSetting{rrrIterSettingData}, setting{settingData}, database{databaseData} {}
	db::Database& database;
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
