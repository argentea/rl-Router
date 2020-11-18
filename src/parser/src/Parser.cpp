#include "Parser.h"
#include "logger/src/Logger.h"

using namespace router::parser;
using namespace router;

int Parser::read(const std::string &lefFile, const std::string &defFile, const std::string &guideFile) {
    logger::Logger::info("Start reading benchmarks");
    _session.init();
    const Rsyn::Json params = {
            {"lefFile", lefFile},
            {"defFile", defFile},
            {"guideFile", guideFile},
    };
    Rsyn::ISPD2018Reader reader;
    reader.load(&_session, params);

    _physical_service = _session.getService("rsyn.physical");
    _routeGuide_service = _session.getService("rsyn.routingGuide");
    _physical_design = _physical_service->getPhysicalDesign();
    _design = _session.getDesign();
    _module = _design.getTopModule();

    logger::Logger::info("Finish reading benchmarks");
    return 0;
}
BoxT<int64_t> Parser::getBoxFromRsynBounds(const Bounds &bounds) {
    return {bounds.getLower().x, bounds.getLower().y, bounds.getUpper().x, bounds.getUpper().y};
}
int Parser::getPinAccessBoxes(Rsyn::PhysicalPort phPort, vector<BoxOnLayer> &accessBoxes) {
    auto displacement = phPort.getPosition();
    auto bounds = phPort.getBounds();
    Bounds dummyCellBounds(displacement, displacement);
    Rsyn::PhysicalTransform transform(dummyCellBounds, phPort.getOrientation());
    bounds.translate(displacement);
    bounds = transform.apply(bounds);
    accessBoxes.emplace_back(phPort.getLayer().getRelativeIndex(), getBoxFromRsynBounds(bounds));
    return 0;
}
int Parser::getPinAccessBoxes(Rsyn::PhysicalLibraryPin phLibPin, Rsyn::PhysicalCell phCell, std::vector<BoxOnLayer> &accessBoxes, const DBUxy &origin) {
    if (!phLibPin.hasPinGeometries()) {
        logger::Logger::warning("pin of " + phCell.getName() + " has no pinGeometries");
        return 0;
    }

    const DBUxy displacement = phCell.getPosition() + origin;
    auto transform = phCell.getTransform();
    // for (Rsyn::PhysicalPinGeometry phPinGeo : phLibPin.allPinGeometries()) {
    // TODO: check why multiple PinGeometry on 8t4 inst60849
    auto phPinGeo = phLibPin.allPinGeometries()[0];
    for (Rsyn::PhysicalPinLayer phPinLayer : phPinGeo.allPinLayers()) {
        if (!phPinLayer.hasRectangleBounds()) {
            logger::Logger::warning("pin has no RectangleBounds");
            continue;
        }
        int layerIdx = phPinLayer.getLayer().getRelativeIndex();
        for (auto bounds : phPinLayer.allBounds()) {
            bounds.translate(displacement);
            bounds = transform.apply(bounds);
            accessBoxes.emplace_back(layerIdx, getBoxFromRsynBounds(bounds));
        }
    }
    return 0;
}
int Parser::initPinAccessBoxes(Rsyn::Pin rsynPin, std::vector<BoxOnLayer> &accessBoxes, int64_t libDBU) {
    // PhysicalPort
    if (rsynPin.isPort()) {
        Rsyn::PhysicalPort phPort = _physical_design.getPhysicalPort(rsynPin.getPort());
        getPinAccessBoxes(phPort, accessBoxes);
        return 0;
    }

    // PhysicalLibraryPin
    Rsyn::PhysicalLibraryPin phLibPin = _physical_design.getPhysicalLibraryPin(rsynPin);

    // PhysicalCell
    Rsyn::Instance instance = rsynPin.getInstance();
    if (instance.getType() != Rsyn::CELL) {
        logger::Logger::warning("pin is not on a cell" + rsynPin.getNetName() + " " + rsynPin.getInstanceName());
        return 0;
    }
    Rsyn::Cell cell = instance.asCell();
    Rsyn::PhysicalCell phCell = _physical_design.getPhysicalCell(cell);
    Rsyn::PhysicalLibraryCell phLibCell = _physical_design.getPhysicalLibraryCell(cell);
    const DBUxy origin(static_cast<DBU>(std::round(phLibCell.getMacro()->originX() * libDBU)),
                       static_cast<DBU>(std::round(phLibCell.getMacro()->originY() * libDBU)));

    // fill accessBoxes
    getPinAccessBoxes(phLibPin, phCell, accessBoxes, origin);
    return 0;
}
int Parser::initNet(Net &net, int i, const Rsyn::Net &rsyn_net) {
    net.idx = i;
    net._net_name = rsyn_net.getName();

    // pins
    net.pinAccessBoxes.reserve(rsyn_net.getNumPins());
    const Rsyn::Session session;
    const Rsyn::PhysicalDesign &physicalDesign = static_cast<Rsyn::PhysicalService *>(_session.getService("rsyn.physical"))->getPhysicalDesign();
    const DBU libDBU = physicalDesign.getDatabaseUnits(Rsyn::LIBRARY_DBU);
    for (auto RsynPin : rsyn_net.allPins()) {
        _rsyn_pins.push_back(RsynPin);
        net.pinAccessBoxes.emplace_back();
        initPinAccessBoxes(RsynPin, net.pinAccessBoxes.back(), libDBU);
    }

    // route guides
    const Rsyn::NetGuide &netGuide = _routeGuide_service->getGuide(rsyn_net);
    for (const Rsyn::LayerGuide &layerGuide : netGuide.allLayerGuides()) {
        auto bounds = layerGuide.getBounds();
        net.routeGuides.emplace_back(layerGuide.getLayer().getRelativeIndex(), getBoxFromRsynBounds(bounds));
    }
    net.routeGuideVios.resize(net.routeGuides.size(), 0);
    return 0;
}

int Parser::initNetlist() {
    logger::Logger::info("Init NetList ...");
    _netlist.nets.reserve(_design.getNumNets());
    int numPins = 0;
    for (Rsyn::Net rsyn_net : _module.allNets()) {
        switch (rsyn_net.getUse()) {
            case Rsyn::POWER:
                continue;
            case Rsyn::GROUND:
                continue;
            default:
                break;
        }
        Net net;
        initNet(net, _netlist.nets.size(), rsyn_net);
        _netlist.nets.emplace_back(net);
        numPins += _netlist.nets.back().pinAccessBoxes.size();
    }
    logger::Logger::info("The number of nets is " + std::to_string(_netlist.nets.size()));
    logger::Logger::info("The number of pins is " + std::to_string(_netlist.nets.size()));
    return 0;
}
