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

    initNetList();
    markPinAndObsOccupancy();
    initLayerList();
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

int Parser::initNetList() {
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
int Parser::markPinAndObsOccupancy() {
    logger::Logger::info("Mark pin & obs occupancy on RouteGrid ...");
    vector<std::pair<BoxOnLayer, int>> fixedMetalVec;

    // STEP 1: get fixed objects
    // Mark pins associated with nets
    for (const auto &net : _netlist.nets) {
        for (const auto &accessBoxes : net.pinAccessBoxes) {
            for (const auto &box : accessBoxes) {
                fixedMetalVec.emplace_back(box, net.idx);
            }
        }
    }
    // Mark dangling pins
    // minor TODO: port?
    const Rsyn::PhysicalDesign &physicalDesign =
            static_cast<Rsyn::PhysicalService *>(_session.getService("rsyn.physical"))->getPhysicalDesign();
    const DBU libDBU = physicalDesign.getDatabaseUnits(Rsyn::LIBRARY_DBU);
    unsigned numUnusedPins = 0;
    unsigned numObs = 0;
    unsigned numSNetObs = 0;
    for (Rsyn::Instance instance : _module.allInstances()) {
        if (instance.getType() != Rsyn::CELL) continue;
        // phCell
        Rsyn::Cell cell = instance.asCell();
        Rsyn::PhysicalCell phCell = _physical_design.getPhysicalCell(cell);
        Rsyn::PhysicalLibraryCell phLibCell = _physical_design.getPhysicalLibraryCell(cell);
        const DBUxy origin(static_cast<DBU>(std::round(phLibCell.getMacro()->originX() * libDBU)),
                           static_cast<DBU>(std::round(phLibCell.getMacro()->originY() * libDBU)));
        // libPin
        for (Rsyn::Pin pin : instance.allPins(false)) {
            if (!pin.getNet()) {// no associated net
                Rsyn::PhysicalLibraryPin phLibPin = _physical_design.getPhysicalLibraryPin(pin);
                vector<BoxOnLayer> accessBoxes;
                getPinAccessBoxes(phLibPin, phCell, accessBoxes, origin);
                for (const auto &box : accessBoxes) {
                    fixedMetalVec.emplace_back(box, OBS_NET_IDX);
                }
                ++numUnusedPins;
            }
        }
        // libObs
        DBUxy displacement = phCell.getPosition() + origin;
        auto transform = phCell.getTransform();
        for (const Rsyn::PhysicalObstacle &phObs : phLibCell.allObstacles()) {
            if (phObs.getLayer().getType() != Rsyn::PhysicalLayerType::ROUTING) continue;
            const int layerIdx = phObs.getLayer().getRelativeIndex();
            for (auto bounds : phObs.allBounds()) {
                bounds.translate(displacement);
                bounds = transform.apply(bounds);
                const BoxOnLayer box(layerIdx, getBoxFromRsynBounds(bounds));
                fixedMetalVec.emplace_back(box, OBS_NET_IDX);
                ++numObs;
            }
        }
    }

    // Mark special nets
    for (Rsyn::PhysicalSpecialNet specialNet : _physical_design.allPhysicalSpecialNets()) {
        for (const DefWireDscp &wire : specialNet.getNet().clsWires) {
            for (const DefWireSegmentDscp &segment : wire.clsWireSegments) {
                int layerIdx =
                        _physical_design.getPhysicalLayerByName(segment.clsLayerName).getRelativeIndex();
                const DBU width = segment.clsRoutedWidth;
                DBUxy pos;
                DBU ext = 0;
                for (unsigned i = 0; i != segment.clsRoutingPoints.size(); ++i) {
                    const DefRoutingPointDscp &pt = segment.clsRoutingPoints[i];
                    const DBUxy &nextPos = pt.clsPos;
                    const DBU nextExt = pt.clsHasExtension ? pt.clsExtension : 0;
                    if (i >= 1) {
                        for (int dim = 0; dim != 2; ++dim) {
                            if (pos[dim] == nextPos[dim]) continue;
                            const DBU l = pos[dim] < nextPos[dim] ? pos[dim] - ext : nextPos[dim] - nextExt;
                            const DBU h = pos[dim] < nextPos[dim] ? nextPos[dim] + nextExt : pos[dim] + ext;
                            BoxOnLayer box(layerIdx);
                            box[dim].Set(l, h);
                            box[1 - dim].Set(pos[1 - dim] - width / 2, pos[1 - dim] + width / 2);
                            fixedMetalVec.emplace_back(box, OBS_NET_IDX);
                            ++numSNetObs;
                            break;
                        }
                    }
                    pos = nextPos;
                    ext = nextExt;
                    if (!pt.clsHasVia) continue;
                    const Rsyn::PhysicalVia &via = _physical_design.getPhysicalViaByName(pt.clsViaName);
                    const int botLayerIdx = via.getBottomLayer().getRelativeIndex();
                    for (const Rsyn::PhysicalViaGeometry &geo : via.allBottomGeometries()) {
                        Bounds bounds = geo.getBounds();
                        bounds.translate(pos);
                        const BoxOnLayer box(botLayerIdx, getBoxFromRsynBounds(bounds));
                        fixedMetalVec.emplace_back(box, OBS_NET_IDX);
                        ++numSNetObs;
                    }
                    const int topLayerIdx = via.getTopLayer().getRelativeIndex();
                    for (const Rsyn::PhysicalViaGeometry &geo : via.allTopGeometries()) {
                        Bounds bounds = geo.getBounds();
                        bounds.translate(pos);
                        const BoxOnLayer box(topLayerIdx, getBoxFromRsynBounds(bounds));
                        fixedMetalVec.emplace_back(box, OBS_NET_IDX);
                        ++numSNetObs;
                    }
                    if (via.hasViaRule()) {
                        const PointT<int> numRowCol =
                                via.hasRowCol() ? PointT<int>(via.getNumCols(), via.getNumRows())
                                                : PointT<int>(1, 1);
                        BoxOnLayer botBox(botLayerIdx);
                        BoxOnLayer topBox(topLayerIdx);
                        for (unsigned dimIdx = 0; dimIdx != 2; ++dimIdx) {
                            const Dimension dim = static_cast<Dimension>(dimIdx);
                            const DBU origin = via.hasOrigin() ? pos[dim] + via.getOrigin(dim) : pos[dim];
                            const DBU botOff =
                                    via.hasOffset() ? origin + via.getOffset(Rsyn::BOTTOM_VIA_LEVEL, dim) : origin;
                            const DBU topOff =
                                    via.hasOffset() ? origin + via.getOffset(Rsyn::TOP_VIA_LEVEL, dim) : origin;
                            const DBU length =
                                    (via.getCutSize(dim) * numRowCol[dim] + via.getSpacing(dim) * (numRowCol[dim] - 1)) / 2;
                            const DBU botEnc = length + via.getEnclosure(Rsyn::BOTTOM_VIA_LEVEL, dim);
                            const DBU topEnc = length + via.getEnclosure(Rsyn::TOP_VIA_LEVEL, dim);
                            botBox[dim].Set(botOff - botEnc, botOff + botEnc);
                            topBox[dim].Set(topOff - topEnc, topOff + topEnc);
                        }
                        fixedMetalVec.emplace_back(botBox, OBS_NET_IDX);
                        fixedMetalVec.emplace_back(topBox, OBS_NET_IDX);
                        numSNetObs += 2;
                    }
                    if (layerIdx == botLayerIdx)
                        layerIdx = topLayerIdx;
                    else if (layerIdx == topLayerIdx)
                        layerIdx = botLayerIdx;
                    else {
                        logger::Logger::error(
                                "Special net " + specialNet.getNet().clsName + " via " + pt.clsViaName + " on wrong layer " + std::to_string(layerIdx));
                        break;
                    }
                }
            }
        }
    }

    return 0;
}
int Parser::initLayerList() {
    Rsyn::PhysicalDesign physicalDesign =
            static_cast<Rsyn::PhysicalService *>(_session.getService("rsyn.physical"))->getPhysicalDesign();
    const DBU libDBU = physicalDesign.getDatabaseUnits(Rsyn::LIBRARY_DBU);

    //  Rsyn::PhysicalLayer (LEF)
    vector<Rsyn::PhysicalLayer> rsynLayers;
    vector<Rsyn::PhysicalLayer> rsynCutLayers;
    for (const Rsyn::PhysicalLayer &rsynLayer : physicalDesign.allPhysicalLayers()) {
        switch (rsynLayer.getType()) {
            case Rsyn::ROUTING:
                rsynLayers.push_back(rsynLayer);
                break;
            case Rsyn::CUT:
                rsynCutLayers.push_back(rsynLayer);
                break;
            default:
                break;
        }
    }
    if (rsynCutLayers.size() + 1 != rsynLayers.size()) {
        logger::Logger::error(
                "rsynCutLayers.size() is " + std::to_string(rsynCutLayers.size()) + " , rsynLayers.size() is " + std::to_string(rsynLayers.size()) + " , not matched... ");
    }

    //  Rsyn::PhysicalVia (LEF)
    vector<vector<Rsyn::PhysicalVia>> rsynVias(rsynCutLayers.size());
    for (const Rsyn::PhysicalVia &rsynVia : physicalDesign.allPhysicalVias()) {
        rsynVias[rsynVia.getCutLayer().getRelativeIndex()].push_back(rsynVia);
    }

    //  Rsyn::PhysicalTracks (DEF)
    vector<Rsyn::PhysicalTracks> rsynTracks(rsynLayers.size());
    for (const Rsyn::PhysicalTracks &rsynTrack : physicalDesign.allPhysicalTracks()) {
        int idx = rsynTrack.allLayers().front().getRelativeIndex();
        if ((rsynTrack.getDirection() == Rsyn::TRACK_HORIZONTAL) ==
            !strcmp(rsynLayers[idx].getLayer()->direction(), "HORIZONTAL")) {
            assert(rsynLayers[idx].getRelativeIndex() == idx);
            rsynTracks[idx] = (rsynTrack);
        }
    }

    // init each MetalLayer
    _metal_layers.clear();
    for (unsigned i = 0; i != rsynLayers.size(); ++i) {
        MetalLayer metal_layer;
        initMetalLayer(metal_layer, rsynLayers[i], physicalDesign.allPhysicalTracks(rsynLayers[i]), libDBU);
        _metal_layers.emplace_back(metal_layer);
    }
    _cut_layers.clear();
    for (unsigned i = 0; i != rsynCutLayers.size(); ++i) {
        CutLayer cut_layer;
        initCutLayer(cut_layer, rsynCutLayers[i], rsynVias[i], _metal_layers[i].direction, _metal_layers[i + 1].direction, libDBU);
        _cut_layers.emplace_back(cut_layer);
    }


    return 0;
}
int Parser::initMetalLayer(MetalLayer &metal_layer, Rsyn::PhysicalLayer rsynLayer, const vector<Rsyn::PhysicalTracks> &rsynTracks, DBU libDBU) {
    // Rsyn::PhysicalLayer (LEF)
    lefiLayer *layer = rsynLayer.getLayer();
    metal_layer.name = layer->name();
    metal_layer.direction = strcmp(layer->direction(), "HORIZONTAL") ? X : Y;
    metal_layer.idx = rsynLayer.getRelativeIndex();
    metal_layer.width = static_cast<DBU>(std::round(layer->width() * libDBU));
    metal_layer.minWidth = static_cast<DBU>(std::round(layer->minwidth() * libDBU));
    metal_layer.widthForSuffOvlp = std::ceil(metal_layer.minWidth * 0.7071);
    metal_layer.shrinkForSuffOvlp = std::max<DBU>(0, std::ceil(metal_layer.widthForSuffOvlp - metal_layer.width * 0.5));
    metal_layer.minArea = static_cast<DBU>(std::round(layer->area() * libDBU * libDBU));
    metal_layer.minLenRaw = metal_layer.minArea / metal_layer.width - metal_layer.width;
    // default spacing
    const int numSpaceTable = layer->numSpacingTable();
    if (!numSpaceTable) {
        logger::Logger::warning(
                "For " + metal_layer.name + ", no run spacing table...");
    } else {
        for (int iSpaceTable = 0; iSpaceTable < numSpaceTable; ++iSpaceTable) {
            if (!layer->spacingTable(iSpaceTable)->isParallel()) {
                logger::Logger::warning(
                        "For " + metal_layer.name + ", unidentified spacing table...");
                continue;
            }

            const lefiParallel *parallel = layer->spacingTable(iSpaceTable)->parallel();
            const int numLength = parallel->numLength();
            if (numLength > 0) {
                metal_layer.parallelLength.resize(numLength);
                for (unsigned iLength = 0; iLength != (unsigned) numLength; ++iLength) {
                    metal_layer.parallelLength[iLength] = static_cast<DBU>(std::round(parallel->length(iLength) * libDBU));
                }
            }
            const int numWidth = parallel->numWidth();
            if (numWidth > 0) {
                metal_layer.parallelWidth.resize(numWidth);
                metal_layer.parallelWidthSpace.resize(numWidth);
                for (unsigned iWidth = 0; iWidth != (unsigned) numWidth; ++iWidth) {
                    metal_layer.parallelWidth[iWidth] = static_cast<DBU>(std::round(parallel->width(iWidth) * libDBU));
                    metal_layer.parallelWidthSpace[iWidth].resize(std::max(1, numLength), 0);
                    for (int iLength = 0; iLength < numLength; ++iLength) {
                        metal_layer.parallelWidthSpace[iWidth][iLength] =
                                static_cast<DBU>(std::round(parallel->widthSpacing(iWidth, iLength) * libDBU));
                    }
                }
                metal_layer.defaultSpace = metal_layer.getParaRunSpace(metal_layer.width);
                metal_layer.paraRunSpaceForLargerWidth = (metal_layer.parallelWidthSpace.size() > 1) ? metal_layer.parallelWidthSpace[1][0] : metal_layer.defaultSpace;
            }
        }
    }
    //  eol spacing
    if (!layer->hasSpacingNumber()) {
        logger::Logger::warning(
                "For " + metal_layer.name + ", no spacing rules...");
    } else {
        const int numSpace = layer->numSpacing();
        metal_layer.spaceRules.reserve(numSpace);
        for (int iSpace = 0; iSpace < numSpace; ++iSpace) {
            const DBU space{std::lround(layer->spacing(iSpace) * libDBU)};
            const DBU eolWidth{std::lround(layer->spacingEolWidth(iSpace) * libDBU)};
            const DBU eolWithin{std::lround(layer->spacingEolWithin(iSpace) * libDBU)};
            const DBU parSpace{std::lround(layer->spacingParSpace(iSpace) * libDBU)};
            const DBU parWithin{std::lround(layer->spacingParWithin(iSpace) * libDBU)};
            if (layer->hasSpacingParellelEdge(iSpace)) {
                metal_layer.spaceRules.emplace_back(space, eolWidth, eolWithin, parSpace, parWithin);
                metal_layer.maxEolSpace = std::max(metal_layer.maxEolSpace, space);
                metal_layer.maxEolWidth = std::max(metal_layer.maxEolWidth, eolWidth);
                metal_layer.maxEolWithin = std::max(metal_layer.maxEolWithin, eolWithin);
            } else if (layer->hasSpacingEndOfLine(iSpace)) {
                metal_layer.spaceRules.emplace_back(space, eolWidth, eolWithin);
                metal_layer.maxEolSpace = std::max(metal_layer.maxEolSpace, space);
                metal_layer.maxEolWidth = std::max(metal_layer.maxEolWidth, eolWidth);
                metal_layer.maxEolWithin = std::max(metal_layer.maxEolWithin, eolWithin);
            } else if (!numSpaceTable) {
                metal_layer.parallelWidthSpace[0][0] = space;
                metal_layer.defaultSpace = metal_layer.getParaRunSpace(metal_layer.width);
            } else if (space != metal_layer.defaultSpace) {
                logger::Logger::warning(
                        "For " + metal_layer.name + ", mismatched defaultSpace & spacingTable...");
            }
        }
        if (metal_layer.spaceRules.empty()) {
            logger::Logger::warning("For " + metal_layer.name + ", no eol spacing rules...");
        }
    }

    for (unsigned iProp = 0; static_cast<int>(iProp) < layer->numProps(); ++iProp) {
        if (!strcmp(layer->propName(iProp), "LEF58_CORNERSPACING")) {
            //  corner spacing
            if (metal_layer.hasCornerSpace()) {
                logger::Logger::warning("For " + metal_layer.name + ", multiple corner spacing rules: " + layer->propValue(iProp) + "...");
                continue;
            }

            std::istringstream iss(layer->propValue(iProp));
            std::string sBuf("");
            double fBuf1{0};
            double fBuf2{0};
            while (iss) {
                iss >> sBuf;
                if (sBuf == "CORNERSPACING" || sBuf == "CONVEXCORNER" || sBuf == ";") {
                    continue;
                }

                if (sBuf == "EXCEPTEOL") {
                    metal_layer.cornerExceptEol = true;
                    iss >> fBuf1;
                    metal_layer.cornerEolWidth = std::lround(fBuf1 * libDBU);
                } else if (sBuf == "WIDTH") {
                    iss >> fBuf1 >> sBuf >> fBuf2;
                    if (fBuf1) {
                        metal_layer.cornerWidth.push_back(std::lround(fBuf1 * libDBU));
                        metal_layer.cornerWidthSpace.push_back(std::lround(fBuf2 * libDBU));
                    } else {
                        metal_layer.cornerWidthSpace[0] = std::lround(fBuf2 * libDBU);
                    }
                } else {
                    logger::Logger::warning("For " + metal_layer.name + ", corner spacing not identified: " + sBuf + "...\n");
                }
            }
        } else if (!strcmp(layer->propName(iProp), "LEF57_SPACING")) {
            //  eol spacing
            std::istringstream iss(layer->propValue(iProp));
            std::string sBuf("");
            double space{0};
            double eolWidth{0};
            double eolWithin{0};
            double parSpace{0};
            double parWithin{0};
            bool hasEol{false};
            bool hasPar{false};
            while (iss) {
                iss >> sBuf;
                if (sBuf == ";") {
                    continue;
                }

                if (sBuf == "SPACING") {
                    iss >> space;
                } else if (sBuf == "ENDOFLINE") {
                    iss >> eolWidth >> sBuf >> eolWithin;
                    hasEol = true;
                } else if (sBuf == "PARALLELEDGE") {
                    iss >> parSpace >> sBuf >> parWithin;
                    hasPar = true;
                } else {
                    logger::Logger::warning("For " + metal_layer.name + ", eol spacing not identified: " + sBuf + "...");
                }
            }
            if (hasPar) {
                metal_layer.spaceRules.emplace_back(std::lround(space * libDBU),
                                                    std::lround(eolWidth),
                                                    std::lround(eolWithin),
                                                    std::lround(parSpace),
                                                    std::lround(parWithin));
                metal_layer.maxEolSpace = std::max(metal_layer.maxEolSpace, std::lround(space * libDBU));
                metal_layer.maxEolWidth = std::max(metal_layer.maxEolWidth, std::lround(eolWidth * libDBU));
                metal_layer.maxEolWithin = std::max(metal_layer.maxEolWithin, std::lround(eolWithin * libDBU));
            } else if (hasEol) {
                metal_layer.spaceRules.emplace_back(std::lround(space * libDBU), std::lround(eolWidth), std::lround(eolWithin));
                metal_layer.maxEolSpace = std::max(metal_layer.maxEolSpace, std::lround(space * libDBU));
                metal_layer.maxEolWidth = std::max(metal_layer.maxEolWidth, std::lround(eolWidth * libDBU));
                metal_layer.maxEolWithin = std::max(metal_layer.maxEolWithin, std::lround(eolWithin * libDBU));
            } else if (!numSpaceTable) {
                metal_layer.parallelWidthSpace[0][0] = std::lround(space * libDBU);
                metal_layer.defaultSpace = metal_layer.getParaRunSpace(metal_layer.width);
            } else if (std::lround(space * libDBU) != metal_layer.defaultSpace) {
                logger::Logger::warning("For " + metal_layer.name + ", mismatched defaultSpace & spacingTable...");
            }
        } else {
            logger::Logger::warning("For " + metal_layer.name + ", unknown prop: " + layer->propName(iProp) + "...");
        }
    }
    metal_layer.fixedMetalQueryMargin = std::max(metal_layer.maxEolSpace, metal_layer.maxEolWithin);

    // Rsyn::PhysicalTracks (DEF)
    // note: crossPoints will be initialized in LayerList
    if (rsynTracks.empty()) {
        logger::Logger::error(
                "For " + metal_layer.name + ", tracks is empty...");
        metal_layer.pitch = metal_layer.width + metal_layer.parallelWidthSpace[0][0];
    } else {
        for (const Rsyn::PhysicalTracks &rsynTrack : rsynTracks) {
            if ((rsynTrack.getDirection() == Rsyn::TRACK_HORIZONTAL) == (metal_layer.direction == X)) {
                continue;
            }
            metal_layer.pitch = rsynTrack.getSpace();
            DBU location = rsynTrack.getLocation();
            for (int i = 0; i < rsynTrack.getNumberOfTracks(); ++i) {
                metal_layer.tracks.emplace_back(location);
                location += metal_layer.pitch;
            }
        }
        sort(metal_layer.tracks.begin(), metal_layer.tracks.end(), [](const Track &lhs, const Track &rhs) { return lhs.location < rhs.location; });
        metal_layer.pitch = metal_layer.tracks[1].location - metal_layer.tracks[0].location;
    }
    delete rsynLayer.getLayer();
    return 0;
}
int Parser::initCutLayer(CutLayer &cut_layer, const Rsyn::PhysicalLayer &rsynLayer, const vector<Rsyn::PhysicalVia> &rsynVias, Direction botDim, Direction topDim, const DBU libDBU) {
    cut_layer.name = rsynLayer.getName();
    cut_layer.idx = rsynLayer.getRelativeIndex();
    cut_layer.width = rsynLayer.getWidth();
    //  Rsyn::PhysicalLayer (LEF)
    const lefiLayer *layer = rsynLayer.getLayer();
    if (layer->hasSpacingNumber() && layer->numSpacing()) {
        cut_layer.spacing = std::lround(layer->spacing(0) * libDBU);
    }

    if (layer->numProps()) {
        for (unsigned iProp = 0; static_cast<int>(iProp) < layer->numProps(); ++iProp) {
            if (!strcmp(layer->propName(iProp), "LEF58_SPACINGTABLE")) {
                std::istringstream iss(layer->propValue(iProp));
                std::string sBuf("");
                double space{0};
                while (iss) {
                    iss >> sBuf;
                    if (sBuf == ";") {
                        continue;
                    }

                    if (sBuf == "DEFAULT") {
                        iss >> space;
                        if (!cut_layer.spacing) {
                            cut_layer.spacing = std::lround(space * libDBU);
                        } else if (std::lround(space * libDBU) != cut_layer.spacing) {
                            logger::Logger::warning("For " + cut_layer.name + ", mismatched defaultSpace & spacingTable... ");
                        }
                    }
                }
            } else {
                logger::Logger::warning("For " + cut_layer.name + ", unknown prop: " + layer->propName(iProp) + "...");
            }
        }
    }

    if (!cut_layer.spacing) {
        logger::Logger::error("For " + cut_layer.name + " CutLayer::init, rsynSpacingRule is empty, init all rules with default 0... ");
    }
    delete rsynLayer.getLayer();

    //  Rsyn::PhysicalVia (LEF)
    if (rsynVias.empty()) {
        logger::Logger::error("For " + cut_layer.name + " rsynVias is empty...");
    }

    int defaultViaTypeIdx = -1;
    const DBU dbuMax =
            std::numeric_limits<DBU>::has_infinity ? std::numeric_limits<DBU>::infinity() : std::numeric_limits<DBU>::max();
    std::tuple<DBU, DBU, DBU, DBU> bestScore(dbuMax, dbuMax, dbuMax, dbuMax);
    for (const Rsyn::PhysicalVia &rsynVia : rsynVias) {
        if (rsynVia.isViaDesign()) {
            continue;
        }

        if ((rsynVia.allBottomGeometries().size() != 1 ||
             rsynVia.allCutGeometries().size() != 1 ||
             rsynVia.allTopGeometries().size() != 1)) {
            logger::Logger::warning("For " + rsynVia.getName() + " , has not exactly one metal layer bound or more than one cut layer bound... ");
        }

        ViaType via_type;
        initViaType(via_type, rsynVia);
        cut_layer.allViaTypes.emplace_back(via_type);
        const std::tuple<DBU, DBU, DBU, DBU> &score = cut_layer.allViaTypes.back().getDefaultScore(botDim, topDim);
        if (score < bestScore) {
            bestScore = score;
            defaultViaTypeIdx = static_cast<int>((cut_layer.allViaTypes.size())) - 1;
        }
    }

    if (defaultViaTypeIdx == -1) {
        logger::Logger::error("For " + cut_layer.name + " all rsyn vias have not exactly one via bound... ");
    }

    // make default via the first one
    if (defaultViaTypeIdx > 0) {
        std::swap(cut_layer.allViaTypes[0], cut_layer.allViaTypes[defaultViaTypeIdx]);
    }
    // init ViaType::idx
    for (unsigned i = 0; i != cut_layer.allViaTypes.size(); ++i) {
        cut_layer.allViaTypes[i].idx = i;
    }

    return 0;
}
BoxT<int64_t> Parser::getBoxFromRsynGeometries(const vector<Rsyn::PhysicalViaGeometry> &geos) {
    BoxT<DBU> box;
    for (const Rsyn::PhysicalViaGeometry &geo : geos) {
        box = box.UnionWith(getBoxFromRsynBounds(geo.getBounds()));
    }
    return box;
}
int Parser::initViaType(ViaType &via_type, Rsyn::PhysicalVia rsynVia) {
    if (rsynVia.allCutGeometries().size() > 1) via_type.hasMultiCut = true;

    via_type.bot = getBoxFromRsynGeometries(rsynVia.allBottomGeometries());
    via_type.cut = getBoxFromRsynGeometries(rsynVia.allCutGeometries());
    via_type.top = getBoxFromRsynGeometries(rsynVia.allTopGeometries());
    via_type.name = rsynVia.getName();

    if (rsynVia.hasRowCol()) {
        const DBU xBotEnc = rsynVia.getEnclosure(Rsyn::BOTTOM_VIA_LEVEL, Dimension(X));
        const DBU yBotEnc = rsynVia.getEnclosure(Rsyn::BOTTOM_VIA_LEVEL, Dimension(Y));
        const DBU xTopEnc = rsynVia.getEnclosure(Rsyn::TOP_VIA_LEVEL, Dimension(X));
        const DBU yTopEnc = rsynVia.getEnclosure(Rsyn::TOP_VIA_LEVEL, Dimension(Y));
        const int numRows = rsynVia.getNumRows();
        const int numCols = rsynVia.getNumCols();
        const DBU xCut = (rsynVia.getCutSize(Dimension(X)) * numCols + rsynVia.getSpacing(Dimension(X)) * (numCols - 1) + 1) / 2;
        const DBU yCut = (rsynVia.getCutSize(Dimension(Y)) * numRows + rsynVia.getSpacing(Dimension(Y)) * (numRows - 1) + 1) / 2;
        via_type.bot = {-xCut - xBotEnc, -yCut - yBotEnc, xCut + xBotEnc, yCut + yBotEnc};
        via_type.cut = {-xCut, -yCut, xCut, yCut};
        via_type.top = {-xCut - xTopEnc, -yCut - yTopEnc, xCut + xTopEnc, yCut + yTopEnc};
    }

    if (!via_type.bot.IsStrictValid() || !via_type.cut.IsStrictValid() || !via_type.top.IsStrictValid()) {
        logger::Logger::warning("For " + rsynVia.getName() + " , has non strict valid via layer bound... ");
    }

    return 0;
}