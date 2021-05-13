#include "SingleNetRouter.h"
#include "PreRoute.h"
#include "RlMazeRoute.h"
#include "UpdateDB.h"
#include "PostRoute.h"

SingleNetRouter::SingleNetRouter(db::Net& databaseNet, db::Setting& settingData, db::RrrIterSetting& rrrIterSettingData)
    : setting{settingData}, rrrIterSetting{rrrIterSettingData}, localNet(databaseNet, settingData, rrrIterSettingData), dbNet(databaseNet), status(db::RouteStatus::SUCC_NORMAL) {}

void SingleNetRouter::preRoute() {
    // Pre-route (obtain proper grid boxes)
    status &= PreRoute(localNet).runIterative();
}

void SingleNetRouter::mazeRoute() {
    // Maze route (working on grid only)
    status &= MazeRoute(localNet, setting, rrrIterSetting).run();
    PostMazeRoute(localNet).run();
}

void SingleNetRouter::commitNetToDB() {
    // Commit net to DB (commit result to DB)
    UpdateDB::commitRouteResult(localNet, dbNet);
}
