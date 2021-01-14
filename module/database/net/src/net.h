#pragma once

#include "parser/src/Net.h"
#include "parser/src/Parser.h"
#include "database/topo/src/topo.h"
#include <cstdlib>
using namespace router::parser;

namespace db {

class NetBase {
public:
    ~NetBase();

    int idx;
    ParserNet parser_net;
    const std::string& getName() const { return parser_net._net_name; }

    // pins
//    std::vector<Rsyn::Pin> rsynPins;
    std::vector<std::vector<BoxOnLayer>> pinAccessBoxes;  // (pinIdx, accessBoxIdx) -> BoxOnLayer
    unsigned numOfPins() const noexcept { return pinAccessBoxes.size(); }
    BoxOnLayer getMaxAccessBox(int pinIdx) const;

    // route guides
    std::vector<BoxOnLayer> routeGuides;
    std::vector<GridBoxOnLayer> gridRouteGuides;

    // on-grid route result
//    std::vector<std::shared_ptr<GridSteiner>> gridTopo;
//    std::vector<std::shared_ptr<GridSteiner>> gridTopo_copy;
//    void postOrderVisitGridTopo(const std::function<void(std::shared_ptr<GridSteiner>)>& visit) const;

    // print
    void printBasics(ostream& os) const;
//    void printResult(ostream& os) const;
//    void print(ostream& os = std::cout) const {
//        printBasics(os);
//        printResult(os);
//    }
};

class Net : public NetBase {
public:
    Net(int i, router::parser::ParserNet net);

    // more route guide information
    std::vector<int> routeGuideVios;
//    RTrees routeGuideRTrees;
    std::vector<int> routeGuideVios_copy;
//    RTrees routeGuideRTrees_copy;
//
    // for initialization
    static void getPinAccessBoxes(Rsyn::PhysicalPort phPort, std::vector<BoxOnLayer>& accessBoxes);
    static void getPinAccessBoxes(Rsyn::PhysicalLibraryPin phLibPin,
                                  Rsyn::PhysicalCell phCell,
                                  std::vector<BoxOnLayer>& accessBoxes,
                                  const DBUxy& origin);

    // final route result
    std::vector<DefWireSegmentDscp> defWireSegments;
    void clearPostRouteResult();
    void clearResult();
    void stash();
    void reset();
};

class NetList {
public:
    std::vector<Net> nets;

    void init(Database db);
    void writeNetTopo(const std::string& filename);
};

}
