#pragma once

#include "Database.h"
#include "GeoPrimitive.h"
#include "rsyn/core/Rsyn.h"
#include "rsyn/io/reader/ISPD2018Reader.h"
#include "rsyn/ispd18/RoutingGuide.h"
#include "rsyn/phy/PhysicalService.h"
#include "rsyn/session/Session.h"
#include <string>


namespace router::parser {


class Parser {
public:
    // \brief read the benchmark
    int read(const std::string &lefFile, const std::string &defFile, const std::string &guideFile);
    Database &getDatabase() { return _database; };
    int64_t getDatabaseUnit() const { return _data_base_unit; };

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

    Database _database;

    static BoxT<int64_t> getBoxFromRsynBounds(const Bounds &bounds);
    static BoxT<int64_t> getBoxFromRsynGeometries(const vector<Rsyn::PhysicalViaGeometry> &geos);

    static int getPinAccessBoxes(Rsyn::PhysicalPort phPort, std::vector<BoxOnLayer> &accessBoxes);
    static int getPinAccessBoxes(Rsyn::PhysicalLibraryPin phLibPin, Rsyn::PhysicalCell phCell,
                                 std::vector<BoxOnLayer> &accessBoxes, const DBUxy &origin);
    int initPinAccessBoxes(Rsyn::Pin rsynPin, std::vector<BoxOnLayer> &accessBoxes, int64_t libDBU);
    int initNet(ParserNet &net, int i, const Rsyn::Net &rsyn_net);
    int initNetList();
    int markPinAndObsOccupancy();

    static int initMetalLayer(MetalLayer &metal_layer, Rsyn::PhysicalLayer rsynLayer,
                              const vector<Rsyn::PhysicalTracks> &rsynTracks,
                              DBU libDBU);
    static int initViaType(ViaType &via_type, Rsyn::PhysicalVia rsynVia);
    static int initCutLayer(CutLayer &cut_layer, const Rsyn::PhysicalLayer &rsynLayer,
                            const vector<Rsyn::PhysicalVia> &rsynVias,
                            Direction botDim,
                            Direction topDim,
                            DBU libDBU);

    int initLayerList();
};
}// namespace router::parser

