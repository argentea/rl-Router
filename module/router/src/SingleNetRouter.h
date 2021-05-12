#pragma once

#include "LocalNet.h"
#include "RlMazeRoute.h"
#include "UpdateDB.h"
#include "PostRoute.h"
#include "PostMazeRoute.h"

class SingleNetRouter {
public:
    LocalNet localNet;
    db::Net& dbNet;

    db::RouteStatus status;

    SingleNetRouter(db::Net& dbNet);

    void preRoute();
    void mazeRoute();
    void commitNetToDB();
};
