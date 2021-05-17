#pragma once

#include "LocalNet.h"

class PreRoute {
public:
	db::Database& database;
    PreRoute(LocalNet& localNetData, db::Database& db) :  database(db), localNet(localNetData) {}

    db::RouteStatus runIterative();

private:
    LocalNet& localNet;

    db::RouteStatus run(int numPitchForGuideExpand);
    void expandGuidesToMargin();
    db::RouteStatus expandGuidesToCoverPins();

    bool checkGuideConnTrack() const;
};
