#pragma once

#include"database/topo/src/topo.h"
#include"parser/src/Parser.h"
using namespace router::parser;

namespace db {

class Database/* : public RouteGrid, public NetList*/ {
public:
    utils::BoxT<DBU> dieRegion;

    void init(Parser parser);
    void clear() {
		//todo
		//RouteGrid::clear();
		return;
	}
    void reset() {
		//todo
		//RouteGrid::reset();
		return;
	}
    void stash() {
		//todo
		//RouteGrid::stash();
		return;
		}

//    void writeDEFWireSegment(Net& dbNet, const utils::PointT<DBU>& u, const utils::PointT<DBU>& v, int layerIdx);
//    void writeDEFVia(Net& dbNet, const utils::PointT<DBU>& point, const ViaType& viaType, int layerIdx);
//    void writeDEFFillRect(Net& dbNet, const utils::BoxT<DBU>& rect, const int layerIdx);
//    void writeDEF(const std::string& filename);

    // get girdPinAccessBoxes
    // TODO: better way to differetiate same-layer and diff-layer girdPinAccessBoxes
//    void getGridPinAccessBoxes(const Net& net, vector<vector<db::GridBoxOnLayer>>& gridPinAccessBoxes) const;

private:

    // mark pin and obstacle occupancy on RouteGrid
    void markPinAndObsOccupancy();
    // mark off-grid vias as obstacles
    void addPinViaMetal(std::vector<std::pair<BoxOnLayer, int>>& fixedMetalVec);

    // init safe margin for multi-thread
    void initMTSafeMargin();

    // slice route guide polygons along track direction
    void sliceRouteGuides();

    // construct RTrees for route guides of each net
    void constructRouteGuideRTrees();
};

}  //   namespace db


