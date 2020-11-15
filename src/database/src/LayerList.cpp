#include "LayerList.h"
#include <fstream>
#include <functional>
//#include "Database.h"

using namespace database;

bool LayerList::isValid(const GridPoint &gridPt) const {
    return gridPt.layerIdx >= 0 && gridPt.layerIdx < layers.size() && layers[gridPt.layerIdx].isValid(gridPt);
}

bool LayerList::isValid(const GridBoxOnLayer &gridBox) const {
    return gridBox.layerIdx >= 0 && gridBox.layerIdx < layers.size() && layers[gridBox.layerIdx].isValid(gridBox);
}

bool LayerList::isValid(const ViaBox &viaBox) const {
    return isValid(viaBox.lower) && isValid(viaBox.upper) &&                                // seperately
           getUpper(viaBox.lower) == viaBox.upper && getLower(viaBox.upper) == viaBox.lower;// consistent
}

BoxOnLayer LayerList::getMetalRectForbidRegion(const BoxOnLayer &metalRect, AggrParaRunSpace aggr) const {
    const MetalLayer &layer = layers[metalRect.layerIdx];
    std::int64_t margin[2];// x, y
    for (int dir = 0; dir < 2; ++dir) {
        margin[dir] = layer.getSpace(metalRect, dir, aggr);
        margin[dir] += layer.width / 2;
    }
    return {metalRect.layerIdx,
            metalRect.lx() - margin[0],
            metalRect.ly() - margin[1],
            metalRect.hx() + margin[0],
            metalRect.hy() + margin[1]};
}

std::vector<utils::BoxT<std::int64_t>> LayerList::getAccurateMetalRectForbidRegions(const BoxOnLayer &metalRect) const {
    const MetalLayer &layer = layers[metalRect.layerIdx];
    std::vector<utils::BoxT<std::int64_t>> results;
    for (int dir = 0; dir < 2; ++dir) {
        const std::int64_t range = metalRect[1 - dir].range();
        if (range < layer.maxEolWidth) {
            utils::BoxT<std::int64_t> region = metalRect;
            region[1 - dir].low -= layer.maxEolWithin;
            region[1 - dir].high += layer.maxEolWithin;
            region[dir].low -= layer.maxEolSpace;
            region[dir].high += layer.maxEolSpace;
            results.push_back(region);
        } else {
            std::int64_t space = layer.getParaRunSpace(metalRect);
            utils::BoxT<std::int64_t> region = metalRect;
            region[dir].low -= space;
            region[dir].high += space;
            results.push_back(region);
        }
    }
    return results;
}

void LayerList::expandBox(BoxOnLayer &box, int numPitchToExtend) const {
    std::int64_t margin = layers[box.layerIdx].pitch * numPitchToExtend;
    box.lx() -= margin;
    box.ly() -= margin;
    box.hx() += margin;
    box.hy() += margin;
}

void LayerList::expandBox(BoxOnLayer &box, int numPitchToExtend, int dir) const {
    std::int64_t margin = layers[box.layerIdx].pitch * numPitchToExtend;
    box[dir].low -= margin;
    box[dir].high += margin;
}

utils::IntervalT<int> LayerList::getSurroundingTrack(int layerIdx, std::int64_t loc) const {
    assert(layerIdx >= 0 && layerIdx < layers.size());
    return layers[layerIdx].getSurroundingTrack(loc);
}

utils::IntervalT<int> LayerList::getSurroundingCrossPoint(int layerIdx, std::int64_t loc) const {
    if (layerIdx == 0) {
        return layers[1].getSurroundingTrack(loc);
    } else if (layerIdx == layers.size() - 1) {
        return layers[layerIdx - 1].getSurroundingTrack(loc);
    } else {
        const utils::IntervalT<int> &upperTrack = layers[layerIdx + 1].getSurroundingTrack(loc);
        const utils::IntervalT<int> &lowerTrack = layers[layerIdx - 1].getSurroundingTrack(loc);
        const utils::IntervalT<int> &fromUpperTrack = layers[layerIdx + 1].getLowerCrossPointRange(upperTrack);
        const utils::IntervalT<int> &fromLowerTrack = layers[layerIdx - 1].getUpperCrossPointRange(lowerTrack);
        return fromUpperTrack.IntersectWith(fromLowerTrack);
    }
}

GridBoxOnLayer LayerList::getSurroundingGrid(int layerIdx, utils::PointT<std::int64_t> loc) const {
    auto dir = layers[layerIdx].direction;
    return {layerIdx, getSurroundingTrack(layerIdx, loc[dir]), getSurroundingCrossPoint(layerIdx, loc[1 - dir])};
}

utils::IntervalT<int> LayerList::rangeSearchTrack(int layerIdx,
                                                  const utils::IntervalT<std::int64_t> &locRange,
                                                  bool includeBound) const {
    assert(layerIdx >= 0 && layerIdx < layers.size());
    return layers[layerIdx].rangeSearchTrack(locRange, includeBound);
}

utils::IntervalT<int> LayerList::rangeSearchCrossPoint(int layerIdx,
                                                       const utils::IntervalT<std::int64_t> &locRange,
                                                       bool includeBound) const {
    assert(layerIdx >= 0 && layerIdx < layers.size());
    if (layerIdx == 0) {
        return layers[1].rangeSearchTrack(locRange, includeBound);
    } else if (layerIdx == layers.size() - 1) {
        return layers[layerIdx - 1].rangeSearchTrack(locRange, includeBound);
    } else {
        const utils::IntervalT<int> &upperTrack = layers[layerIdx + 1].rangeSearchTrack(locRange, includeBound);
        const utils::IntervalT<int> &lowerTrack = layers[layerIdx - 1].rangeSearchTrack(locRange, includeBound);
        bool upperValid = layers[layerIdx + 1].isTrackRangeValid(upperTrack);
        bool lowerValid = layers[layerIdx - 1].isTrackRangeValid(lowerTrack);
        if (upperValid && lowerValid) {
            // the most typical case
            const utils::IntervalT<int> &fromUpperTrack = layers[layerIdx + 1].getLowerCrossPointRange(upperTrack);
            const utils::IntervalT<int> &fromLowerTrack = layers[layerIdx - 1].getUpperCrossPointRange(lowerTrack);
            return fromUpperTrack.UnionWith(fromLowerTrack);
        } else if (upperValid || (!lowerValid && layers[layerIdx + 1].isTrackRangeWeaklyValid(upperTrack))) {
            return layers[layerIdx + 1].getLowerCrossPointRange(upperTrack);
        } else if (layers[layerIdx - 1].isTrackRangeWeaklyValid(lowerTrack)) {
            return layers[layerIdx - 1].getUpperCrossPointRange(lowerTrack);
        } else {
            return {0, -1};// a little bit safer than default {inf, -inf}
        }
    }
}

GridBoxOnLayer LayerList::rangeSearch(const BoxOnLayer &box, bool includeBound) const {
    auto dir = layers[box.layerIdx].direction;
    return {box.layerIdx,
            rangeSearchTrack(box.layerIdx, box[dir], includeBound),
            rangeSearchCrossPoint(box.layerIdx, box[1 - dir], includeBound)};
}

utils::PointT<std::int64_t> LayerList::getLoc(const GridPoint &gridPt) const {
    assert(isValid(gridPt));
    return layers[gridPt.layerIdx].getLoc(gridPt);
}

BoxOnLayer LayerList::getLoc(const GridBoxOnLayer &gridBox) const {
    assert(isValid(gridBox));
    return layers[gridBox.layerIdx].getLoc(gridBox);
}

std::pair<utils::PointT<std::int64_t>, utils::PointT<std::int64_t>> LayerList::getLoc(const GridEdge &edge) const {
    assert(edge.isTrackSegment() || edge.isWrongWaySegment());
    return layers[edge.u.layerIdx].getLoc(edge);
}

GridPoint LayerList::getUpper(const GridPoint &cur) const {
    assert(isValid(cur) && cur.layerIdx < (int) layers.size() - 1);
    return layers[cur.layerIdx].getUpper(cur);
}

GridPoint LayerList::getLower(const GridPoint &cur) const {
    assert(isValid(cur) && cur.layerIdx > 0);
    return layers[cur.layerIdx].getLower(cur);
}

GridBoxOnLayer LayerList::getUpper(const GridBoxOnLayer &cur) const {
    assert(isValid(cur) && cur.layerIdx < (int) layers.size() - 1);
    return GridBoxOnLayer(
            // layer
            cur.layerIdx + 1,
            // track
            layers[cur.layerIdx + 1].rangeSearchTrack(
                    {layers[cur.layerIdx].crossPoints[cur.crossPointRange.low].location,
                     layers[cur.layerIdx].crossPoints[cur.crossPointRange.high].location}),
            // cross point
            layers[cur.layerIdx].getUpperCrossPointRange(cur.trackRange));
}

GridBoxOnLayer LayerList::getLower(const GridBoxOnLayer &cur) const {
    assert(isValid(cur) && cur.layerIdx > 0);
    return GridBoxOnLayer(
            // layer
            cur.layerIdx - 1,
            // track
            layers[cur.layerIdx - 1].rangeSearchTrack(
                    {layers[cur.layerIdx].crossPoints[cur.crossPointRange.low].location,
                     layers[cur.layerIdx].crossPoints[cur.crossPointRange.high].location}),
            // cross point
            layers[cur.layerIdx].getLowerCrossPointRange(cur.trackRange));
}

ViaBox LayerList::getViaBoxBetween(const BoxOnLayer &lower, const BoxOnLayer &upper) const {
    assert((lower.layerIdx + 1) == upper.layerIdx);
    auto box2d = lower.IntersectWith(upper);
    auto lowerGridBoxTmp = rangeSearch({lower.layerIdx, box2d});
    if (!isValid(lowerGridBoxTmp)) return ViaBox();// invalid
    auto upperGridBox = getUpper(lowerGridBoxTmp);
    if (!isValid(upperGridBox)) return ViaBox();// invalid
    auto lowerGridBox = getLower(upperGridBox);
    if (!isValid(lowerGridBox)) return ViaBox();// invalid
    return ViaBox(lowerGridBox, upperGridBox);
}

bool LayerList::isConnected(const GridBoxOnLayer &lhs, const GridBoxOnLayer &rhs) {
    if (!isValid(lhs) || !isValid(rhs)) {
        return false;
    } else if ((lhs.layerIdx + 1) == rhs.layerIdx) {
        return isValid(getViaBoxBetween(lhs, rhs));
    } else if (lhs.layerIdx == (rhs.layerIdx + 1)) {
        return isValid(getViaBoxBetween(rhs, lhs));
    } else if (lhs.layerIdx == rhs.layerIdx) {
        return lhs.trackRange.HasIntersectWith(rhs.trackRange) &&
               lhs.crossPointRange.HasIntersectWith(rhs.crossPointRange);
    } else {
        return false;
    }
}

bool LayerList::isAdjacent(const GridBoxOnLayer &lhs, const GridBoxOnLayer &rhs) {
    if (!isValid(lhs) || !isValid(rhs)) {
        return false;
    } else if (lhs.layerIdx == rhs.layerIdx) {
        return (abs(lhs.trackRange.low - rhs.trackRange.high) == 1 ||
                abs(rhs.trackRange.low - lhs.trackRange.high) == 1) &&
               lhs.crossPointRange.HasIntersectWith(rhs.crossPointRange);
    } else {
        return false;
    }
}

void LayerList::initCrossPoints() {
    for (unsigned i = 0; i != layers.size(); ++i) {
        std::vector<CrossPoint> &crossPoints = layers[i].crossPoints;
        std::vector<Track> emptyTrackSet;
        std::vector<Track> &lowerTrackSet = (i > 0) ? layers[i - 1].tracks : emptyTrackSet;
        std::vector<Track> &upperTrackSet = (i < (layers.size() - 1)) ? layers[i + 1].tracks : emptyTrackSet;

        // merge cross points to lower and upper layers
        int iLo = 0, iUp = 0;// track indexes
        std::int64_t lastBoth = 0;
        while (iLo < lowerTrackSet.size() || iUp < upperTrackSet.size()) {
            if (iUp >= upperTrackSet.size()) {
                crossPoints.emplace_back(lowerTrackSet[iLo].location, iLo, -1);
                lowerTrackSet[iLo].upperCPIdx = crossPoints.size() - 1;
                ++iLo;
            } else if (iLo >= lowerTrackSet.size()) {
                crossPoints.emplace_back(upperTrackSet[iUp].location, -1, iUp);
                upperTrackSet[iUp].lowerCPIdx = crossPoints.size() - 1;
                ++iUp;
            }// boundaries should be checked first
            else if (lowerTrackSet[iLo].location < upperTrackSet[iUp].location) {
                crossPoints.emplace_back(lowerTrackSet[iLo].location, iLo, -1);
                lowerTrackSet[iLo].upperCPIdx = crossPoints.size() - 1;
                ++iLo;
            } else if (lowerTrackSet[iLo].location > upperTrackSet[iUp].location) {
                crossPoints.emplace_back(upperTrackSet[iUp].location, -1, iUp);
                upperTrackSet[iUp].lowerCPIdx = crossPoints.size() - 1;
                ++iUp;
            } else {// iLo < lowerTrackSet.size() && iUp < lowerTrackSet.size() && lowerTrackSet[iLo].location ==
                // lowerTrackSet[iUp].location
                crossPoints.emplace_back(lowerTrackSet[iLo].location, iLo, iUp);
                lowerTrackSet[iLo].upperCPIdx = crossPoints.size() - 1;
                upperTrackSet[iUp].lowerCPIdx = crossPoints.size() - 1;
                ++iLo;
                ++iUp;
            }
        }

        layers[i].initAccCrossPointDistCost();
    }
}

void LayerList::initOppLUT(const std::vector<std::vector<std::vector<bool>>> &ori, std::vector<std::vector<std::vector<bool>>> &opp) {
    const size_t nCPs = ori.size();
    size_t xSize = 0;
    for (const std::vector<std::vector<bool>> &orig : ori) {
        xSize = orig.size();
        if (xSize) {
            break;
        }
    }

    auto travelCPs = [&](std::function<void(const unsigned, const unsigned, const unsigned, const unsigned)> handle) {
        for (unsigned i = 0; i != nCPs; ++i) {
            if (ori[i].empty()) {
                continue;
            }
            for (unsigned j = 0; j != xSize; ++j) {
                const int ySize = ((int) ori[i][0].size() - 1) / 2;
                for (unsigned k = std::max(0, ySize - (int) i); k < std::min(ySize * 2 + 1, int(nCPs + ySize - i)); ++k) {
                    if (ori[i][j][k]) {
                        handle(i, j, ySize, k);
                    }
                }
            }
        }
    };

    std::vector<int> ySizes(nCPs, -1);
    auto updateSize = [&](const unsigned i, const unsigned j, const unsigned ySize, const unsigned k) {
        ySizes[i + k - ySize] = std::max(ySizes[i + k - ySize], abs((int) ySize - (int) k));
    };
    auto fillTable = [&](const unsigned i, const unsigned j, const unsigned ySize, const unsigned k) {
        std::vector<bool> &tmpOpp = opp[i + k - ySize][xSize - j - 1];
        const int tmpYSize = ((int) tmpOpp.size() - 1) / 2;
        tmpOpp[tmpYSize + ySize - k] = true;
    };

    travelCPs(updateSize);
    opp.clear();
    opp.resize(nCPs);
    for (unsigned i = 0; i != nCPs; ++i) {
        if (ySizes[i] >= 0) {
            opp[i].resize(xSize, std::vector<bool>(ySizes[i] * 2 + 1, false));
        } else if (ori[i].empty()) {
            opp[i].resize(xSize, std::vector<bool>(1, false));
        }
        assert(ori[i].size() + opp[i].size());
    }
    travelCPs(fillTable);
}

void LayerList::initViaWire(const int layerIdx,
                            const utils::BoxT<std::int64_t> &viaMetal,
                            std::vector<std::vector<std::vector<bool>>> &viaWireLUT) {
    const MetalLayer &layer = layers[layerIdx];
    const std::int64_t halfWidth = ceil(layer.width / 2.0);
    const std::int64_t viaMetalWidth = viaMetal[layer.direction].range();
    const std::int64_t viaMetalHeight = viaMetal[1 - layer.direction].range();
    const std::int64_t pSpace = layer.getParaRunSpace(viaMetal);
    const std::int64_t space = std::max({pSpace, layer.maxEolSpace, layer.maxEolWithin});
    const size_t xSize = std::max(ceil((space + halfWidth + viaMetal[layer.direction].high) / (double) layer.pitch),
                                  ceil((space + halfWidth - viaMetal[layer.direction].low) / (double) layer.pitch)) -
                         1;
    const utils::IntervalT<std::int64_t> locRange(-space - halfWidth + viaMetal[1 - layer.direction].low + 1,
                                                  +space + halfWidth + viaMetal[1 - layer.direction].high - 1);
    viaWireLUT.resize(layer.numCrossPoints());
    std::vector<bool> viaTrack(xSize + 1, false);
    for (unsigned i = 0; i != layer.numCrossPoints(); ++i) {
        const CrossPoint &cp = layer.crossPoints[i];
        utils::IntervalT<std::int64_t> tmpLocRange(locRange);
        tmpLocRange.ShiftBy(layer.crossPoints[i].location);
        const utils::IntervalT<int> &cpRange = rangeSearchCrossPoint(layerIdx, tmpLocRange);
        const size_t ySize = std::max((int) i - cpRange.low, cpRange.high - (int) i);
        viaWireLUT[i].resize(xSize * 2 + 1, std::vector<bool>(ySize * 2 + 1, false));
        for (unsigned j = 0; j != xSize * 2 + 1; ++j) {
            std::int64_t xDist = 0;
            if (j < xSize) {
                xDist = std::max(0L, int(xSize - j) * layer.pitch - halfWidth + viaMetal[layer.direction].low);
            } else if (j > xSize) {
                xDist = std::max(0L, int(j - xSize) * layer.pitch - halfWidth - viaMetal[layer.direction].high);
            }
            for (unsigned k = std::max(0, (int) ySize - (int) i); k < std::min(ySize * 2 + 1, layer.numCrossPoints() + ySize - i);
                 ++k) {
                const CrossPoint &tmpCP = layer.crossPoints[i + k - ySize];
                std::int64_t yDist = 0;
                if (k < ySize) {
                    yDist = std::max(0L, cp.location - tmpCP.location - halfWidth + viaMetal[1 - layer.direction].low);
                } else if (k > ySize) {
                    yDist = std::max(0L, tmpCP.location - cp.location - halfWidth - viaMetal[1 - layer.direction].high);
                }
                if (pow(xDist, 2) + pow(yDist, 2) < pow(pSpace, 2) ||
                    layer.isEolViolation(xDist, viaMetalHeight, yDist) ||
                    layer.isEolViolation(yDist, viaMetalWidth, xDist)) {
                    viaTrack[abs((int) j - (int) xSize)] = true;
                    viaWireLUT[i][j][k] = true;
                }
            }
        }
    }
    size_t minXSize = xSize;
    for (; minXSize && !viaTrack[minXSize]; --minXSize) {
    }
    unsigned d = xSize - minXSize;
    if (!d) {
        return;
    }
    for (std::vector<std::vector<bool>> &vvb : viaWireLUT) {
        for (unsigned i = 0; i != minXSize * 2 + 1; ++i) {
            vvb[i] = vvb[i + d];
        }
        vvb.resize(minXSize * 2 + 1);
    }
}

void LayerList::initViaConfLUT() {
    for (unsigned i = 0; i != cutLayers.size(); ++i) {
        CutLayer &cutLayer = cutLayers[i];
        // Loops for init all-all via-via LUTs
        for (unsigned j = 0; j != cutLayer.allViaTypes.size(); ++j) {
            ViaType &viaType1 = cutLayer.allViaTypes[j];
            viaType1.allViaCut.resize(cutLayer.allViaTypes.size());
            viaType1.allViaMetal.resize(cutLayer.allViaTypes.size());
            viaType1.allViaMetalNum.resize(cutLayer.allViaTypes.size());
            for (unsigned k = 0; k != cutLayer.allViaTypes.size(); ++k) {
                ViaType &viaType2 = cutLayer.allViaTypes[k];
                auto &viaCut = viaType1.allViaCut[k];
                auto &viaMetal = viaType1.allViaMetal[k];
                auto &viaMetalNum = viaType1.allViaMetalNum[k];
                initSameLayerViaConfLUT(i, viaType1, viaType2, viaCut, viaMetal, viaMetalNum);
            }
        }

        //  2. init viaTopVia & viaBotVia
        if (i > 0) {
            // Loops for init all-all via-via LUTs
            for (unsigned j = 0; j != cutLayer.allViaTypes.size(); ++j) {
                ViaType &viaType1 = cutLayer.allViaTypes[j];
                viaType1.allViaBotVia.resize(cutLayers[i - 1].allViaTypes.size());
                for (unsigned k = 0; k != cutLayers[i - 1].allViaTypes.size(); ++k) {
                    ViaType &viaType2 = cutLayers[i - 1].allViaTypes[k];
                    viaType2.allViaTopVia.resize(cutLayer.allViaTypes.size());
                    auto &viaBotVia = viaType1.allViaBotVia[k];
                    auto &viaTopVia = viaType2.allViaTopVia[j];
                    initDiffLayerViaConfLUT(i, viaType1, viaType2, viaBotVia, viaTopVia);
                }
            }
        }

        //  3. init viaBotWire & viaTopWire
        layers[i].wireTopVia.resize(cutLayer.allViaTypes.size());
        layers[i + 1].wireBotVia.resize(cutLayer.allViaTypes.size());
        for (auto &viaType : cutLayer.allViaTypes) {
            initViaWire(i, viaType.bot, viaType.viaBotWire);
            initViaWire(i + 1, viaType.top, viaType.viaTopWire);
            LayerList::initOppLUT(viaType.viaBotWire, layers[i].wireTopVia[viaType.idx]);
            LayerList::initOppLUT(viaType.viaTopWire, layers[i + 1].wireBotVia[viaType.idx]);
        }
    }

    // 4. init wireRange
    for (MetalLayer &layer : layers) {
        layer.initWireRange();
    }

    // Merge LUTs
    for (unsigned i = 0; i != cutLayers.size(); ++i) {
        CutLayer &cutLayer = cutLayers[i];
        for (unsigned j = 0; j != cutLayer.allViaTypes.size(); ++j) {
            ViaType &viaType = cutLayer.allViaTypes[j];
            viaType.mergedAllViaMetal = mergeLUTs(viaType.allViaMetal);
            if (i > 0) {
                for (int k = 0; k != viaType.allViaBotVia.size(); ++k) {
                    viaType.mergedAllViaBotVia = mergeLUTsCP(viaType.allViaBotVia);
                }
            }
            if (i < cutLayers.size()) {
                for (int k = 0; k != viaType.allViaTopVia.size(); ++k) {
                    viaType.mergedAllViaTopVia = mergeLUTsCP(viaType.allViaTopVia);
                }
            }
        }
    }
    for (int i = 0; i < layers.size(); ++i) {
        MetalLayer &layer = layers[i];
        if (i > 0) {
            layer.mergedWireBotVia = mergeLUTsCP(layer.wireBotVia);
        }
        if ((i + 1) < layers.size()) {
            layer.mergedWireTopVia = mergeLUTsCP(layer.wireTopVia);
        }
    }

    // Set isWireViaMultiTrack
    for (int i = 0; i < layers.size(); ++i) {
        if (i > 0 && layers[i].wireBotVia[0][0].size() > 1 ||
            (i + 1) < layers.size() && layers[i].wireTopVia[0][0].size() > 1)
            layers[i].isWireViaMultiTrack = true;
    }

    //  writeDefConflictLUTs("debugConflictLUTa.log");
    //  exit(0);
}

void LayerList::initSameLayerViaConfLUT(const int layerIdx,
                                        ViaType &viaT1,
                                        ViaType &viaT2,
                                        std::vector<std::vector<bool>> &viaCut,
                                        std::vector<std::vector<bool>> &viaMetal,
                                        std::vector<std::vector<int>> &viaMetalNum) {
    CutLayer &cutLayer = cutLayers[layerIdx];
    MetalLayer &botLayer = layers[layerIdx];
    MetalLayer &topLayer = layers[layerIdx + 1];
    const Dimension botDim = botLayer.direction;
    const Dimension topDim = topLayer.direction;
    const std::int64_t cutSpacing = cutLayer.spacing;

    const utils::BoxT<std::int64_t> &botT1 = viaT1.bot;
    const utils::BoxT<std::int64_t> &cutT1 = viaT1.cut;
    const utils::BoxT<std::int64_t> &topT1 = viaT1.top;
    const utils::BoxT<std::int64_t> &botT2 = viaT2.bot;
    const utils::BoxT<std::int64_t> &cutT2 = viaT2.cut;
    const utils::BoxT<std::int64_t> &topT2 = viaT2.top;

    const std::int64_t botPSpace = std::max(botLayer.getParaRunSpace(botT1), botLayer.getParaRunSpace(botT2));
    const std::int64_t topPSpace = std::max(topLayer.getParaRunSpace(topT1), topLayer.getParaRunSpace(topT2));
    const std::int64_t botCSpace = 0;// std::max(botLayer.getCornerSpace(botT1), botLayer.getCornerSpace(botT2));
    const std::int64_t topCSpace = 0;// std::max(topLayer.getCornerSpace(topT1), topLayer.getCornerSpace(topT2));
    const std::int64_t botSpace = std::max({botPSpace, botCSpace, botLayer.maxEolSpace, botLayer.maxEolWithin});
    const std::int64_t topSpace = std::max({topPSpace, topCSpace, topLayer.maxEolSpace, topLayer.maxEolWithin});
    const std::int64_t botPitch = botLayer.pitch;
    const std::int64_t topPitch = topLayer.pitch;
    // init viaCut & viaMetal
    const size_t cutXSize = std::max(ceil((cutSpacing + cutT1[botDim].high - cutT2[botDim].low) / (double) botPitch),
                                     ceil((cutSpacing + cutT2[botDim].high - cutT1[botDim].low) / (double) botPitch)) -
                            1;
    const size_t cutYSize = std::max(ceil((cutSpacing + cutT1[topDim].high - cutT2[topDim].low) / (double) topPitch),
                                     ceil((cutSpacing + cutT2[topDim].high - cutT1[topDim].low) / (double) topPitch)) -
                            1;
    const std::int64_t metalXLength = std::max({botSpace + botT2[botDim].high - botT1[botDim].low,
                                                botSpace + botT1[botDim].high - botT2[botDim].low,
                                                topSpace + topT2[botDim].high - topT1[botDim].low,
                                                topSpace + topT1[botDim].high - topT2[botDim].low});
    const std::int64_t metalYLength = std::max({botSpace + botT2[topDim].high - botT1[topDim].low,
                                                botSpace + botT1[topDim].high - botT2[topDim].low,
                                                topSpace + topT2[topDim].high - topT1[topDim].low,
                                                topSpace + topT1[topDim].high - topT2[topDim].low});
    const size_t metalXSize = std::max(cutXSize, (size_t) ceil((metalXLength) / (double) botPitch) - 1);
    const size_t metalYSize = std::max(cutYSize, (size_t) ceil((metalYLength) / (double) topPitch) - 1);
    const std::int64_t maxLength = std::max(metalXLength, metalYLength);
    botLayer.confLutMargin = std::max(botLayer.confLutMargin, maxLength);
    topLayer.confLutMargin = std::max(topLayer.confLutMargin, maxLength);
    viaCut.resize(2 * cutXSize + 1, std::vector<bool>(2 * cutYSize + 1, false));
    viaMetal.resize(2 * metalXSize + 1, std::vector<bool>(2 * metalYSize + 1, false));
    viaMetalNum.resize(2 * metalXSize + 1, std::vector<int>(2 * metalYSize + 1, 0));
    std::vector<bool> viaMetalTrack(2 * metalXSize + 1, false);

    utils::PointT<std::int64_t> delta;
    for (unsigned j = 0; j != 2 * cutXSize + 1; ++j) {
        delta[botDim] = botPitch * (static_cast<int>(j) - static_cast<int>(cutXSize));
        for (unsigned k = 0; k != 2 * cutYSize + 1; ++k) {
            delta[topDim] = topPitch * (static_cast<int>(k) - static_cast<int>(cutYSize));
            utils::BoxT<std::int64_t> tmpT2(cutT2);
            tmpT2.ShiftBy(delta);
            if (utils::L2Dist(tmpT2, cutT1) < cutSpacing) viaCut[j][k] = true;
        }
    }

    const size_t offsetX{metalXSize - cutXSize};
    const size_t offsetY{metalYSize - cutYSize};
    for (unsigned j = 0; j != 2 * metalXSize + 1; ++j) {
        delta[botDim] = botPitch * (static_cast<int>(j) - static_cast<int>(metalXSize));
        for (unsigned k = 0; k != 2 * metalYSize + 1; ++k) {
            delta[topDim] = topPitch * (static_cast<int>(k) - static_cast<int>(metalYSize));
            utils::BoxT<std::int64_t> tmpBotT2(botT2);
            utils::BoxT<std::int64_t> tmpTopT2(topT2);
            tmpBotT2.ShiftBy(delta);
            tmpTopT2.ShiftBy(delta);
            viaMetalNum[j][k] +=
                    static_cast<int>(j <= 2 * cutXSize + offsetX && k <= 2 * cutYSize + offsetY && j >= offsetX &&
                                     k >= offsetY && viaCut[j - offsetX][k - offsetY]) +
                    static_cast<int>(
                            L2Dist(tmpBotT2, botT1) < botPSpace || L2Dist(tmpTopT2, topT1) < topPSpace ||
                            botLayer.isEolViolation(tmpBotT2, botT1) || topLayer.isEolViolation(tmpTopT2, topT1));// ||
                                                                                                                  // utils::ParaRunLength(tmpBotT2, botT1) <= 0 && utils::LInfDist(tmpBotT2, botT1) < botCSpace ||
                                                                                                                  // utils::ParaRunLength(tmpTopT2, topT1) <= 0 && utils::LInfDist(tmpTopT2, topT1) < topCSpace);
            if (viaMetalNum[j][k]) {
                viaMetalTrack[j] = true;
                viaMetal[j][k] = true;
            }
        }
    }

    size_t minMetalXSize = 2 * metalXSize + 1;
    for (; minMetalXSize && !viaMetalTrack[minMetalXSize]; --minMetalXSize) {
    }
    if (minMetalXSize < metalXSize) viaMetal.resize(minMetalXSize + 1);
}

void LayerList::initDiffLayerViaConfLUT(const int layerIdx,
                                        ViaType &viaT1,
                                        ViaType &viaT2,
                                        std::vector<std::vector<std::vector<bool>>> &viaBotVia,
                                        std::vector<std::vector<std::vector<bool>>> &viaTopVia) {
    MetalLayer &botLayer = layers[layerIdx];
    const Dimension botDim = botLayer.direction;

    const utils::BoxT<std::int64_t> &botT1 = viaT1.bot;
    const utils::BoxT<std::int64_t> &topT2 = viaT2.top;

    const std::int64_t botPSpace = std::max(botLayer.getParaRunSpace(botT1), botLayer.getParaRunSpace(topT2));
    const std::int64_t botCSpace = 0;// std::max(botLayer.getCornerSpace(botT1), botLayer.getCornerSpace(topT2));
    const std::int64_t botSpace = std::max({botPSpace, botCSpace, botLayer.maxEolSpace, botLayer.maxEolWithin});
    const std::int64_t botPitch = botLayer.pitch;

    const unsigned nBotCPs = botLayer.numCrossPoints();

    const std::int64_t xLength = botSpace + std::max(botT1[botDim].high - topT2[botDim].low, topT2[botDim].high - botT1[botDim].low);
    const size_t xSize = ceil(xLength / (double) botPitch) - 1;
    const utils::IntervalT<std::int64_t> botLocRange(-topT2[1 - botDim].high + botT1[1 - botDim].low - botSpace + 1,
                                                     +botT1[1 - botDim].high - topT2[1 - botDim].low + botSpace - 1);

    const std::int64_t maxLength = std::max<std::int64_t>({xLength, -botLocRange.low, botLocRange.high});
    botLayer.confLutMargin = std::max(botLayer.confLutMargin, maxLength);
    viaBotVia.resize(nBotCPs);
    for (unsigned j = 0; j != nBotCPs; ++j) {
        utils::IntervalT<std::int64_t> tmpLocRange(botLocRange);
        tmpLocRange.ShiftBy(botLayer.crossPoints[j].location);
        const utils::IntervalT<int> &cpRange = rangeSearchCrossPoint(layerIdx, tmpLocRange);
        const size_t ySize = std::max((int) j - cpRange.low, cpRange.high - (int) j);
        if (botLayer.crossPoints[j].upperTrackIdx >= 0) {
            viaBotVia[j].resize(xSize * 2 + 1, std::vector<bool>(ySize * 2 + 1, false));
        } else {
            viaBotVia[j].resize(xSize * 2 + 1, std::vector<bool>(ySize * 2 + 1, true));
        }
    }

    utils::PointT<std::int64_t> delta;
    for (unsigned j = 0; j != nBotCPs; ++j) {
        const CrossPoint &cp = botLayer.crossPoints[j];
        if (cp.upperTrackIdx == -1) {
            continue;
        }
        const unsigned ySize = (viaBotVia[j][0].size() - 1) / 2;
        for (unsigned k = 0; k != xSize * 2 + 1; ++k) {
            delta[botDim] = botPitch * (static_cast<int>(k) - static_cast<int>(xSize));
            for (unsigned l = std::max(0, (int) ySize - (int) j); l < std::min(ySize * 2 + 1, nBotCPs + ySize - j); ++l) {
                const CrossPoint &tmpCP = botLayer.crossPoints[j + l - ySize];
                if (tmpCP.lowerTrackIdx == -1) {
                    viaBotVia[j][k][l] = true;
                    continue;
                }
                delta[1 - botDim] = tmpCP.location - cp.location;
                utils::BoxT<std::int64_t> tmpTopT2(topT2);
                tmpTopT2.ShiftBy(delta);
                if (L2Dist(tmpTopT2, botT1) < botPSpace || botLayer.isEolViolation(tmpTopT2, botT1)) {// ||
                    // utils::ParaRunLength(tmpTopT2, botT1) <= 0 && utils::LInfDist(tmpTopT2, botT1) < botCSpace) {
                    viaBotVia[j][k][l] = true;
                }
            }
        }
    }
    LayerList::initOppLUT(viaBotVia, viaTopVia);
}

void LayerList::initViaForbidRegions() {
    for (int i = 0; i < cutLayers.size(); ++i) {
        auto &cutLayer = cutLayers[i];
        cutLayer.botMaxForbidRegion = cutLayer.defaultViaType().bot;
        cutLayer.topMaxForbidRegion = cutLayer.defaultViaType().top;
        for (auto &viaType : cutLayer.allViaTypes) {
            viaType.botForbidRegions = getAccurateMetalRectForbidRegions({i, viaType.bot});
            for (const auto &region : viaType.botForbidRegions) {
                cutLayer.botMaxForbidRegion = cutLayer.botMaxForbidRegion.UnionWith(region);
            }
            viaType.topForbidRegions = getAccurateMetalRectForbidRegions({i + 1, viaType.top});
            for (const auto &region : viaType.topForbidRegions) {
                cutLayer.topMaxForbidRegion = cutLayer.topMaxForbidRegion.UnionWith(region);
            }
        }
    }
}

void LayerList::mergeLUT(std::vector<std::vector<bool>> &lhs, const std::vector<std::vector<bool>> &rhs) {
    const unsigned lhsXSize = lhs.size() / 2;
    const unsigned lhsYSize = lhs[0].size() / 2;
    const unsigned rhsXSize = rhs.size() / 2;
    const unsigned rhsYSize = rhs[0].size() / 2;
    const unsigned offsetX = lhsXSize - rhsXSize;
    const unsigned offsetY = lhsYSize - rhsYSize;
    for (unsigned j = 0; j != rhs.size(); ++j) {
        for (unsigned k = 0; k != rhs[0].size(); ++k) {
            if (rhs[j][k]) lhs[j + offsetX][k + offsetY] = true;
        }
    }
}

std::vector<std::vector<bool>> LayerList::mergeLUTs(const std::vector<std::vector<std::vector<bool>>> &LUTs) {
    int XSize = 0, YSize = 0;
    std::vector<std::vector<bool>> mergedLUT;
    for (auto &LUT : LUTs) {
        XSize = std::max(XSize, int(LUT.size()));
        YSize = std::max(YSize, int(LUT[0].size()));
    }
    mergedLUT.resize(XSize, std::vector<bool>(YSize, false));
    for (auto &LUT : LUTs) {
        mergeLUT(mergedLUT, LUT);
    }
    return mergedLUT;
}

std::vector<std::vector<std::vector<bool>>> LayerList::mergeLUTsCP(const std::vector<std::vector<std::vector<std::vector<bool>>>> &LUTs) {
    std::vector<int> XSizes(LUTs[0].size(), 0);
    std::vector<int> YSizes(LUTs[0].size(), 0);
    // crossPointIdx, trackIdx, crossPointIdx
    std::vector<std::vector<std::vector<bool>>> mergedLUTs(LUTs[0].size());
    for (unsigned cpIdx = 0; cpIdx != LUTs[0].size(); ++cpIdx) {
        for (unsigned typeIdx = 0; typeIdx != LUTs.size(); ++typeIdx) {
            XSizes[cpIdx] = std::max(XSizes[cpIdx], int(LUTs[typeIdx][cpIdx].size()));
            YSizes[cpIdx] = std::max(YSizes[cpIdx], int(LUTs[typeIdx][cpIdx][0].size()));
        }
        mergedLUTs[cpIdx].resize(XSizes[cpIdx], std::vector<bool>(YSizes[cpIdx], false));
    }

    for (unsigned cpIdx = 0; cpIdx != LUTs[0].size(); ++cpIdx) {
        for (unsigned typeIdx = 0; typeIdx != LUTs.size(); ++typeIdx) {
            mergeLUT(mergedLUTs[cpIdx], LUTs[typeIdx][cpIdx]);
        }
    }

    return mergedLUTs;
}


void LayerList::writeDefConflictLUTs(const std::string &debugFileName) const {
    std::ofstream ofs(debugFileName);
    int cpIdx = 0;
    for (const auto &cutLayer : cutLayers) {
        //        cutLayer.printBasics(ofs);
        //        cutLayer.printDesignRules(ofs);
        //        cutLayer.printViaOccupancyLUT(ofs);
        ofs << "viaCut" << std::endl;
        int xSize = ((int) cutLayer.allViaTypes[0].allViaCut[0].size() - 1) / 2;
        int ySize = ((int) cutLayer.allViaTypes[0].allViaCut[0][0].size() - 1) / 2;
        for (int j = xSize; j != cutLayer.allViaTypes[0].allViaCut[0].size(); ++j) {
            for (int k = ySize; k != cutLayer.allViaTypes[0].allViaCut[0][0].size(); ++k) {
                ofs << (int) (cutLayer.allViaTypes[0].allViaCut[0][j][k]) << " ";
            }
            ofs << std::endl;
        }
        ofs << std::endl;
        ofs << "viaMetal" << std::endl;
        xSize = ((int) cutLayer.allViaTypes[0].allViaMetal[0].size() - 1) / 2;
        ySize = ((int) cutLayer.allViaTypes[0].allViaMetal[0][0].size() - 1) / 2;
        for (int j = xSize; j != cutLayer.allViaTypes[0].allViaMetal[0].size(); ++j) {
            for (int k = ySize; k != cutLayer.allViaTypes[0].allViaMetal[0][0].size(); ++k) {
                ofs << (int) (cutLayer.allViaTypes[0].allViaMetal[0][j][k]) << " ";
            }
            ofs << std::endl;
        }
        ofs << std::endl;
        ofs << "viaBotVia" << std::endl;
        cpIdx = 0;
        if (cutLayer.idx > 0) {
            for (const auto &cp : cutLayer.allViaTypes[0].allViaBotVia[0]) {
                ofs << "cpidx is: " << cpIdx++ << std::endl;
                for (auto a : cp) {
                    for (auto b : a) {
                        ofs << (int) (b) << " ";
                    }
                    ofs << std::endl;
                }
            }
        }
        ofs << "viaTopVia" << std::endl;
        cpIdx = 0;
        if (cutLayer.idx < cutLayers.size() - 1) {
            for (const auto &cp : cutLayer.allViaTypes[0].allViaTopVia[0]) {
                ofs << "cpidx is: " << cpIdx++ << std::endl;
                for (auto a : cp) {
                    for (auto b : a) {
                        ofs << (int) (b) << " ";
                    }
                    ofs << std::endl;
                }
            }
        }
        ofs << std::endl;
    }
}
