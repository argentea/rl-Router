#pragma once

#include "LocalNet.h"
#include "RlMazeRoute.h"
#include "UpdateDB.h"
#include "PostRoute.h"
#include "PostMazeRoute.h"
#include <Setting.h>

class SingleNetRouter {
public:
    LocalNet localNet;
	db::Setting& setting;
	db::RrrIterSetting& rrrIterSetting;
    db::Net& dbNet;

    db::RouteStatus status;

    SingleNetRouter(db::Net& dbNet, db::Setting& settingData, db::RrrIterSetting& rrrIterSettingData);

    void preRoute();
    void mazeRoute();
    void commitNetToDB();
};
