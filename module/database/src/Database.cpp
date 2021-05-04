#include "Database.h"
#include "database/grid/src/Setting.h"
#include "parser/src/Parser.h"
#include "logger/src/Logger.h"

namespace db {

void Database::init(Parser& parser, Setting& settingData) {
	

	setting = settingData;
//    auto dieBound = rsynService.physicalDesign.getPhysicalDie().getBounds();
//    dieRegion = getBoxFromRsynBounds(dieBound);
    RouteGrid::init(parser);

    NetList::init(parser.getDatabase());

    markPinAndObsOccupancy();

    initMTSafeMargin();

    sliceRouteGuides();

    constructRouteGuideRTrees();

	std::cout << "Finish initializing database" << std::endl;
	std::cout << "MEM: cur=" << utils::mem_use::get_current() << "MB, peak=" << utils::mem_use::get_peak() << "MB"
          << std::endl;
  	std::cout << std::endl;
}


void Database::markPinAndObsOccupancy(Parser& parser) {
    if (setting.dbVerbose >= +db::VerboseLevelT::MIDDLE) {
        log() << "Mark pin & obs occupancy on RouteGrid ..." << std::endl;
    }
    // STEP 2: mark
    if (setting.dbVerbose >= +db::VerboseLevelT::MIDDLE) {
        printlog("mark fixed metal rtrees...");
    }

    markFixedMetalBatch(fixedMetalVec, 0, fixedMetalVec.size());

    addPinViaMetal(fixedMetalVec);

    // Mark poor wire
    if (setting.dbVerbose >= +db::VerboseLevelT::MIDDLE) {
        printlog("mark poor wire map...");
    }
    for (const auto& fixedMetal : fixedMetalVec) {
        const auto& fixedBox = fixedMetal.first;
        AggrParaRunSpace aggr = AggrParaRunSpace::DEFAULT;
        if (getLayer(0).parallelLength.size() <= 1) {
            // hack for ISPD'18 test cases
            aggr = AggrParaRunSpace::LARGER_WIDTH;
            if (min(fixedBox.width(), fixedBox.height()) == getLayer(fixedBox.layerIdx).width &&
                getOvlpFixedMetals(fixedBox, NULL_NET_IDX).size() == 1) {
                aggr = AggrParaRunSpace::DEFAULT;
            }
        } else {
            // hack for ISPD'19 test cases
            aggr = AggrParaRunSpace::LARGER_LENGTH;
        }
        auto fixedForbidRegion = getMetalRectForbidRegion(fixedBox, aggr);
        auto gridBox = rangeSearch(fixedForbidRegion, aggr == AggrParaRunSpace::LARGER_WIDTH);  // TODO: change to false
        if (!isValid(gridBox)) continue;
        for (int trackIdx = gridBox.trackRange.low; trackIdx <= gridBox.trackRange.high; ++trackIdx) {
            usePoorWireSegment({gridBox.layerIdx, trackIdx, gridBox.crossPointRange}, fixedMetal.second);
        }
        if (fixedMetal.second >=0) {
            // add initial hist cost to help pin access
            if (gridBox.layerIdx != 0) {
                useHistWireSegments(getLower(gridBox), fixedMetal.second, setting.dbInitHistUsageForPinAccess);
            }
            if (gridBox.layerIdx != getLayerNum() - 1) {
                useHistWireSegments(getUpper(gridBox), fixedMetal.second, setting.dbInitHistUsageForPinAccess);
            }
        }
    }
    // Mark poor via
    for (int i = 0; i < getLayerNum() - 1; ++i) {
        usePoorViaMap[i] = (layerNumFixedObjects[i] >= setting.dbUsePoorViaMapThres ||
                            layerNumFixedObjects[i + 1] >= setting.dbUsePoorViaMapThres);
    }

    initPoorViaMap(fixedMetalVec);
}

void Database::addPinViaMetal(vector<std::pair<BoxOnLayer, int>>& fixedMetalVec) {
    usePoorViaMap.resize(getLayerNum() - 1, false);
    int beginIdx = fixedMetalVec.size();

    std::mutex metalMutex;
    auto pinViaMT = runJobsMT(database.nets.size(), [&](int netIdx) { 
        const auto& net = database.nets[netIdx];
        vector<vector<db::GridBoxOnLayer>> gridPinAccessBoxes;
        getGridPinAccessBoxes(net, gridPinAccessBoxes);
        for (int pinIdx = 0; pinIdx < net.numOfPins(); pinIdx++) {
            const auto& accessBoxes = gridPinAccessBoxes[pinIdx];
            if (accessBoxes.size() <= 1) continue;
            int lastIdx = accessBoxes.size() - 1;
            if (accessBoxes[lastIdx].layerIdx == accessBoxes[lastIdx - 1].layerIdx) continue;
            // handle diff-layer gridBox
            const auto& gridBox = accessBoxes[lastIdx];
            db::GridPoint tap(gridBox.layerIdx, gridBox.trackRange.low, gridBox.crossPointRange.low);
            db::BoxOnLayer bestBox;
            db::RouteStatus status =
                PinTapConnector::getBestPinAccessBox(getLoc(tap), tap.layerIdx, net.pinAccessBoxes[pinIdx], bestBox);
            if (status != +db::RouteStatus::SUCC_CONN_EXT_PIN || bestBox.layerIdx == tap.layerIdx) continue;
            utils::PointT<DBU> viaLoc(bestBox.cx(), bestBox.cy());
            int layerIdx = min(bestBox.layerIdx, tap.layerIdx);
            auto viaType = database.getBestViaTypeForFixed(viaLoc, layerIdx, net.idx);
            metalMutex.lock();
            fixedMetalVec.emplace_back(db::BoxOnLayer(layerIdx, viaType.getShiftedBotMetal(viaLoc)), net.idx);
            fixedMetalVec.emplace_back(db::BoxOnLayer(layerIdx + 1, viaType.getShiftedTopMetal(viaLoc)), net.idx);
            metalMutex.unlock();
        }
    });
    if (db::setting.dbVerbose >= +db::VerboseLevelT::MIDDLE) {
        printlog("pinViaMT", pinViaMT);
    }

    markFixedMetalBatch(fixedMetalVec, beginIdx, fixedMetalVec.size());  // TODO: may not be needed
}

void Database::initMTSafeMargin() {
    for (auto& layer : layers) {
        layer.mtSafeMargin = max({layer.minAreaMargin, layer.confLutMargin, layer.fixedMetalQueryMargin});
        if (db::setting.dbVerbose >= +db::VerboseLevelT::MIDDLE) {
            printlog(layer.name, "mtSafeMargin = max {", layer.minAreaMargin, layer.confLutMargin, layer.fixedMetalQueryMargin, "} =", layer.mtSafeMargin);
        }
    }
}

void Database::sliceRouteGuides() {
    if (db::setting.dbVerbose >= +db::VerboseLevelT::MIDDLE) {
        log() << "Slice RouteGuides ..." << std::endl;
        log() << std::endl;
    }
    for (auto& net : nets) {
        vector<vector<utils::BoxT<DBU>>> guides(getLayerNum());  // route guides on different layers
        for (auto& guide : net.routeGuides) {
            guides[guide.layerIdx].push_back(guide);
        }
        net.routeGuides.clear();
        for (int layerIdx = 0; layerIdx < getLayerNum(); ++layerIdx) {
            utils::SlicePolygons<DBU>(guides[layerIdx], 1 - getLayerDir(layerIdx));
            for (const auto& guide : guides[layerIdx]) {
                net.routeGuides.emplace_back(layerIdx, guide);
                net.gridRouteGuides.push_back(rangeSearch(net.routeGuides.back()));
            }
        }
    }
}

void Database::constructRouteGuideRTrees() {
    if (db::setting.dbVerbose >= +db::VerboseLevelT::MIDDLE) {
        log() << "Construct r-trees for route guides of each net ..." << std::endl;
        log() << std::endl;
    }
    for (auto& net : nets) {
        net.routeGuideRTrees.resize(getLayerNum());
        vector<vector<std::pair<boostBox, int>>> rtreeItems;
        rtreeItems.resize(getLayerNum());
        for (unsigned i = 0; i < net.routeGuides.size(); ++i) {
            const auto& guide = net.routeGuides[i];
            boostBox b(boostPoint(guide.x.low, guide.y.low), boostPoint(guide.x.high, guide.y.high));
            rtreeItems[guide.layerIdx].push_back(std::make_pair(b, i));
            // net.routeGuideRTrees[guide.layerIdx].insert(std::make_pair(b, i));
        }
        for (int layerIdx = 0; layerIdx < getLayerNum(); layerIdx++) {
            RTree tRtree(rtreeItems[layerIdx]);
            net.routeGuideRTrees[layerIdx] = boost::move(tRtree);
        }
    }
}

void Database::writeDEFWireSegment(Net& dbNet, const utils::PointT<DBU>& u, const utils::PointT<DBU>& v, int layerIdx) {
    dbNet.defWireSegments.emplace_back();
    DefWireSegmentDscp& segment = dbNet.defWireSegments.back();

    segment.clsRoutingPoints.resize(2);
    segment.clsRoutingPoints[0].clsPos.set(u.x, u.y);
    segment.clsRoutingPoints[1].clsPos.set(v.x, v.y);
    segment.clsLayerName = getLayer(layerIdx).name;
}

void Database::writeDEFVia(Net& dbNet, const utils::PointT<DBU>& point, const ViaType& viaType, int layerIdx) {
    dbNet.defWireSegments.emplace_back();
    DefWireSegmentDscp& segment = dbNet.defWireSegments.back();

    segment.clsRoutingPoints.resize(1);
    segment.clsRoutingPoints[0].clsPos.set(point.x, point.y);
    segment.clsRoutingPoints[0].clsHasVia = true;
    segment.clsLayerName = getLayer(layerIdx).name;
    segment.clsRoutingPoints[0].clsViaName = viaType.name;
}

void Database::writeDEFFillRect(Net& dbNet, const utils::BoxT<DBU>& rect, const int layerIdx) {
    dbNet.defWireSegments.emplace_back();
    DefWireSegmentDscp& segment = dbNet.defWireSegments.back();

    segment.clsRoutingPoints.resize(1);
    segment.clsRoutingPoints[0].clsPos.set(rect.lx(), rect.ly());
    segment.clsRoutingPoints[0].clsHasRectangle = true;
    segment.clsRoutingPoints[0].clsRect = {0, 0, rect.hx() - rect.lx(), rect.hy() - rect.ly()};
    segment.clsLayerName = getLayer(layerIdx).name;
}

void Database::getGridPinAccessBoxes(const Net& net, vector<vector<db::GridBoxOnLayer>>& gridPinAccessBoxes) const {
    gridPinAccessBoxes.resize(net.numOfPins());
    for (unsigned pinIdx = 0; pinIdx != net.numOfPins(); ++pinIdx) {
        vector<vector<db::GridBoxOnLayer>> pins(getLayerNum());
        for (const db::BoxOnLayer& pinAccessBox : net.pinAccessBoxes[pinIdx]) {
            int dir = getLayerDir(pinAccessBox.layerIdx);
            DBU pitch = getLayer(pinAccessBox.layerIdx).pitch;
            // pinForbidRegion
            auto pinForbidRegion = getMetalRectForbidRegion(pinAccessBox, AggrParaRunSpace::DEFAULT);
            const db::GridBoxOnLayer& gridPinForbidRegion = rangeSearch(pinForbidRegion);
            if (isValid(gridPinForbidRegion)) {
                pins[pinAccessBox.layerIdx].push_back(gridPinForbidRegion);
            }
            // One-pitch extension
            auto pinExtension = pinAccessBox;
            for (int d = 0; d < 2; ++d) {
                pinExtension[d].low -= pitch;
                pinExtension[d].high += pitch;
            }
            const db::GridBoxOnLayer& gridPinExtension = rangeSearch(pinExtension);
            for (int trackIdx = gridPinExtension.trackRange.low; trackIdx <= gridPinExtension.trackRange.high;
                 ++trackIdx) {
                for (int cpIdx = gridPinExtension.crossPointRange.low; cpIdx <= gridPinExtension.crossPointRange.high;
                     ++cpIdx) {
                    db::GridPoint pt(pinAccessBox.layerIdx, trackIdx, cpIdx);
                    if (!gridPinForbidRegion.includePoint(pt) && Dist(pinAccessBox, getLoc(pt)) <= pitch) {
                        pins[pinAccessBox.layerIdx].emplace_back(pinAccessBox.layerIdx,
                                                                 utils::IntervalT<int>{trackIdx, trackIdx},
                                                                 utils::IntervalT<int>{cpIdx, cpIdx});
                    }
                }
            }
        }

        // assign a relatively far grid access box if none (rarely happen)
        unsigned numBoxes = 0;
        for (const vector<db::GridBoxOnLayer>& pin : pins) {
            numBoxes += pin.size();
        }
        if (!numBoxes) {
            for (const db::BoxOnLayer& pinAccessBox : net.pinAccessBoxes[pinIdx]) {
                db::GridBoxOnLayer gridBox = rangeSearch(pinAccessBox);
                if (gridBox.trackRange.low > gridBox.trackRange.high) {
                    if (gridBox.trackRange.low == 0) {
                        gridBox.trackRange.high = 0;
                    } else {
                        gridBox.trackRange.low = gridBox.trackRange.high;
                    }
                }
                if (gridBox.crossPointRange.low > gridBox.crossPointRange.high) {
                    if (gridBox.crossPointRange.low == 0) {
                        gridBox.crossPointRange.high = 0;
                    } else {
                        gridBox.crossPointRange.low = gridBox.crossPointRange.high;
                    }
                }
                pins[pinAccessBox.layerIdx].push_back(gridBox);
            }
        }

        // slice
        gridPinAccessBoxes[pinIdx].clear();
        for (vector<db::GridBoxOnLayer>& pin : pins) {
            if (!pin.empty()) {
                db::GridBoxOnLayer::sliceGridPolygons(pin);
                for (const db::GridBoxOnLayer& box : pin) {
                    if (isValid(box)) {
                        gridPinAccessBoxes[pinIdx].push_back(box);
                    }
                }
            }
        }

        // assign diff-layer access point if all poor
		// todo

        bool allPinTapPoor = true;
        for (auto& gridBox : gridPinAccessBoxes[pinIdx]) {
            for (int trackIdx = gridBox.trackRange.low; trackIdx <= gridBox.trackRange.high; ++trackIdx) {
                for (int cpIdx = gridBox.crossPointRange.low; cpIdx <= gridBox.crossPointRange.high; ++cpIdx) {
                    db::GridPoint upper_pt(gridBox.layerIdx, trackIdx, cpIdx);
                    if ((gridBox.layerIdx == getLayerNum() - 1 || !isValid(getUpper(upper_pt)) ||
                         getViaPoorness(upper_pt, net.idx) != db::RouteGrid::ViaPoorness::Poor) &&
                        (gridBox.layerIdx == 0 || !isValid(getLower(upper_pt)) ||
                         getViaPoorness(getLower(upper_pt), net.idx) != db::RouteGrid::ViaPoorness::Poor)) {
                        allPinTapPoor = false;
                        break;
                    }
                }
                if (!allPinTapPoor) break;
            }
            if (!allPinTapPoor) break;
        }
		
        if (allPinTapPoor) {
            auto bestBox = net.getMaxAccessBox(pinIdx);
            auto addDiffLayerGridPinAccessBox = [&](const BoxOnLayer& pinBox) {
                auto gridBox =
                    getSurroundingGrid(pinBox.layerIdx, utils::PointT<DBU>(pinBox.x.center(), pinBox.y.center()));
                gridPinAccessBoxes[pinIdx].push_back(gridBox);
            };
            if (bestBox.layerIdx > 0) {
                auto pinBox = bestBox;
                pinBox.layerIdx--;
                addDiffLayerGridPinAccessBox(pinBox);
            }
            if (bestBox.layerIdx < getLayerNum() - 1) {
                auto pinBox = bestBox;
                pinBox.layerIdx++;
                addDiffLayerGridPinAccessBox(pinBox);
            }
        }
		
    }
}

}  // namespace db


