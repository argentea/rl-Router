#ifndef RL_ROUTER_PARSER_H
#define RL_ROUTER_PARSER_H

#include "CutLayer.h"
#include "GeoPrimitive.h"
#include "MetalLayer.h"
#include "Net.h"
#include "rsyn/core/Rsyn.h"
#include "rsyn/io/reader/ISPD2018Reader.h"
#include "rsyn/ispd18/RoutingGuide.h"
#include "rsyn/phy/PhysicalService.h"
#include "rsyn/session/Session.h"
#include <string>


namespace router {
namespace parser {


class Parser {
public:
    // \brief read the benchmark
    // \return 0: success
    int read(const std::string &lefFile, const std::string &defFile, const std::string &guideFile);

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
    NetList _netlist;
    std::vector<MetalLayer> _metal_layers;
    std::vector<CutLayer> _cut_layers;

    static BoxT<int64_t> getBoxFromRsynBounds(const Bounds &bounds);
    static BoxT<int64_t> getBoxFromRsynGeometries(const vector<Rsyn::PhysicalViaGeometry>& geos);

    static int getPinAccessBoxes(Rsyn::PhysicalPort phPort, std::vector<BoxOnLayer> &accessBoxes);
    static int getPinAccessBoxes(Rsyn::PhysicalLibraryPin phLibPin, Rsyn::PhysicalCell phCell,
                                 std::vector<BoxOnLayer> &accessBoxes, const DBUxy &origin);
    int initPinAccessBoxes(Rsyn::Pin rsynPin, std::vector<BoxOnLayer> &accessBoxes, int64_t libDBU);
    int initNet(Net &net, int i, const Rsyn::Net &rsyn_net);
    int initNetList();
    int markPinAndObsOccupancy();

    int initMetalLayer(MetalLayer &metal_layer, Rsyn::PhysicalLayer rsynLayer,
                       const vector<Rsyn::PhysicalTracks> &rsynTracks,
                       DBU libDBU);
    static int initViaType(ViaType &via_type, Rsyn::PhysicalVia rsynVia);
    int initCutLayer(CutLayer &cut_layer, const Rsyn::PhysicalLayer &rsynLayer,
                 const vector<Rsyn::PhysicalVia> &rsynVias,
                 Direction botDim,
                 Direction topDim,
                 DBU libDBU);

    int initLayerList();
};
}// namespace parser
}// namespace router

#endif//RL_ROUTER_PARSER_H
