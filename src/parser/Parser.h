#ifndef RL_ROUTER_PARSER_H
#define RL_ROUTER_PARSER_H

#include "database/src/GeoPrimitive.h"
#include "database/src/Net.h"
#include "rsyn/core/Rsyn.h"
#include "rsyn/io/reader/ISPD2018Reader.h"
#include "rsyn/ispd18/RoutingGuide.h"
#include "rsyn/phy/PhysicalService.h"
#include "rsyn/session/Session.h"
#include <string>
#include <vector>

namespace parser {
class Parser {
public:
    // \brief read the benchmark
    // \return 0: success
    int read(const std::string &lefFile, const std::string &defFile, const std::string &guideFile);
    int64_t getDatabaseUnit() const { return _data_base_unit; };
    int initNetlist(database::NetList &netlist);

private:
    int64_t _data_base_unit = 0;
    Rsyn::Session _session;

    Rsyn::PhysicalService *_physical_service;
    Rsyn::RoutingGuide *_routeGuide_service;
    Rsyn::PhysicalDesign _physical_design;
    Rsyn::Design _design;
    Rsyn::Module _module;

    Rsyn::Net _rsyn_net;
    std::vector<Rsyn::Pin> _rsyn_pins;
    utils::BoxT<DBU> getBoxFromRsynBounds(const Bounds &bounds);
    int getPinAccessBoxes(Rsyn::PhysicalPort phPort, std::vector<database::BoxOnLayer> &accessBoxes);

    int getPinAccessBoxes(Rsyn::PhysicalLibraryPin phLibPin, Rsyn::PhysicalCell phCell,
                          std::vector<database::BoxOnLayer> &accessBoxes, const DBUxy &origin);
    int initPinAccessBoxes(Rsyn::Pin rsynPin, std::vector<database::BoxOnLayer> &accessBoxes, std::int64_t libDBU);
    int initNet(database::Net &net, int i, const Rsyn::Net &rsyn_net);
};
}// namespace parser

#endif// RL_ROUTER_PARSER_H
