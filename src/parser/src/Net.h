#pragma once

#include "GridTopo.h"
#include <boost/geometry.hpp>
#include <boost/geometry/geometries/box.hpp>
#include <boost/geometry/geometries/point.hpp>
#include <boost/geometry/index/rtree.hpp>

namespace router {
namespace parser {

namespace bgi = boost::geometry::index;
namespace bg = boost::geometry;
using boostPoint = bg::model::point<std::int64_t, 2, bg::cs::cartesian>;
using boostBox = bg::model::box<boostPoint>;
using RTrees = std::vector<bgi::rtree<std::pair<boostBox, int>, bgi::rstar<32>>>;

class Database;

class NetBase {
public:
    ~NetBase();

    int idx;
    std::string _net_name;
    const std::string &getName() const { return _net_name; }

    // pins
    //    vector<Rsyn::Pin> rsynPins;
    std::vector<std::vector<BoxOnLayer>> pinAccessBoxes;// (pinIdx, accessBoxIdx) -> BoxOnLayer
    unsigned numOfPins() const noexcept { return pinAccessBoxes.size(); }
    BoxOnLayer getMaxAccessBox(int pinIdx) const;

    // route guides
    std::vector<BoxOnLayer> routeGuides;
    std::vector<GridBoxOnLayer> gridRouteGuides;

    // on-grid route result
    std::vector<std::shared_ptr<GridSteiner>> gridTopo;
    std::vector<std::shared_ptr<GridSteiner>> gridTopo_copy;
    void postOrderVisitGridTopo(const std::function<void(std::shared_ptr<GridSteiner>)> &visit) const;
};

class Net : public NetBase {
public:
    // more route guide information
    std::vector<int> routeGuideVios;
    RTrees routeGuideRTrees;
    std::vector<int> routeGuideVios_copy;
    RTrees routeGuideRTrees_copy;


    // final route result
    //    vector<DefWireSegmentDscp> defWireSegments;
    //    void clearPostRouteResult();
    void clearResult();
    void stash();
    void reset();
};

class NetList {
public:
    std::vector<Net> nets;
};

}// namespace parser
}// namespace router
