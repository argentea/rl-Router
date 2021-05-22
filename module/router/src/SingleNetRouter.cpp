#include "SingleNetRouter.h"
#include "PreRoute.h"
#include "RlMazeRoute.h"
#include "UpdateDB.h"
#include "PostRoute.h"

SingleNetRouter::SingleNetRouter(db::Net& databaseNet, db::Setting& settingData, db::RrrIterSetting& rrrIterSettingData, db::Database& db)
    : localNet(databaseNet, settingData, rrrIterSettingData, db), setting{settingData}, rrrIterSetting{rrrIterSettingData}, dbNet(databaseNet), database(db), status(db::RouteStatus::SUCC_NORMAL) {}

void SingleNetRouter::preRoute() {
    // Pre-route (obtain proper grid boxes)
    status &= PreRoute(localNet, database).runIterative();
}

void SingleNetRouter::mazeRoute() {
    // Maze route (working on grid only)
    status &= MazeRoute(localNet, setting, rrrIterSetting, database).run();
    PostMazeRoute(localNet, database).run();
}

void SingleNetRouter::commitNetToDB() {
    // Commit net to DB (commit result to DB)
    UpdateDB::commitRouteResult(localNet, dbNet, database);
}
