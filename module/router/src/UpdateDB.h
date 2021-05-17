#pragma once

#include "LocalNet.h"

class UpdateDB {
public:
    // Note: after commitRouteResult, localNet should not be used (as move())
    static void commitRouteResult(LocalNet& localNet, db::Net& dbNet, db::Database& database);
    static void clearRouteResult(db::Net& dbNet, db::Database& database);
    static void commitMinAreaRouteResult(db::Net& dbNet, db::Database& database);
    static void clearMinAreaRouteResult(db::Net& dbNet, db::Database& database);
    static void commitViaTypes(db::Net& dbNet, db::Database& database);
    static bool checkViolation(db::Net& dbNet, db::RrrIterSetting rrrIterSetting, db::Database& database);
    static double getNetVioCost(const db::Net &dbNet, db::Database& database);
};
