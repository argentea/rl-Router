#include "MetalLayer.h"
using namespace database;
//ostream& operator<<(ostream& os, const Track& track) {
//    os << "track(lo=" << track.lowerCPIdx << ", up=" << track.upperCPIdx << ", loc=" << track.location << ")";
//    return os;
//}
//
//ostream& operator<<(ostream& os, const CrossPoint& cp) {
//    os << "crossPt(lo=" << cp.lowerTrackIdx << ", up=" << cp.upperTrackIdx << ", loc=" << cp.location << ")";
//    return os;
//}

bool MetalLayer::isTrackRangeValid(const utils::IntervalT<int> &trackRange) const {
    return trackRange.low >= 0 && trackRange.high < tracks.size() && trackRange.IsValid();
}

bool MetalLayer::isTrackRangeWeaklyValid(const utils::IntervalT<int> &trackRange) const {
    return trackRange.low >= 0 && trackRange.low < tracks.size() && trackRange.high >= 0 &&
           trackRange.high < tracks.size();
}

utils::IntervalT<int> MetalLayer::getUpperCrossPointRange(const utils::IntervalT<int> &trackRange) const {
    return {tracks[trackRange.low].upperCPIdx, tracks[trackRange.high].upperCPIdx};
}

utils::IntervalT<int> MetalLayer::getLowerCrossPointRange(const utils::IntervalT<int> &trackRange) const {
    return {tracks[trackRange.low].lowerCPIdx, tracks[trackRange.high].lowerCPIdx};
}

utils::IntervalT<int> MetalLayer::getSurroundingTrack(std::int64_t loc) const {
    // offset by firstTrackLoc(), make it within track range, and round
    const double floatingTrackIdx =
            (std::min(std::max(firstTrackLoc(), loc), lastTrackLoc()) - firstTrackLoc()) / static_cast<double>(pitch);
    return {floor(floatingTrackIdx), ceil(floatingTrackIdx)};
}

utils::IntervalT<int> MetalLayer::rangeSearchTrack(const utils::IntervalT<std::int64_t> &locRange, bool includeBound) const {
    auto locRangeCopy = locRange;
    // invalid range (low >= high) will still be invalid
    if (locRangeCopy.low < firstTrackLoc()) {
        locRangeCopy.low = firstTrackLoc();
    }
    if (locRangeCopy.high > lastTrackLoc()) {
        locRangeCopy.high = lastTrackLoc();
    }

    utils::IntervalT<int> res{ceil(double(locRangeCopy.low - firstTrackLoc()) / double(pitch)),
                              floor(double(locRangeCopy.high - firstTrackLoc()) / double(pitch))};

    if (!includeBound) {
        if (res.high >= 0 && res.high < numTracks() && tracks[res.high].location == locRange.high) --res.high;
        if (res.low >= 0 && res.low < numTracks() && tracks[res.low].location == locRange.low) ++res.low;
    }

    return res;
}

bool MetalLayer::isCrossPointRangeValid(const utils::IntervalT<int> &crossPointRange) const {
    return crossPointRange.low >= 0 && crossPointRange.high < crossPoints.size() && crossPointRange.IsValid();
}

void MetalLayer::initAccCrossPointDistCost() {
    accCrossPointDistCost.resize(numCrossPoints() + 1);
    accCrossPointDistCost[0] = 0;
    // For four uniform-distributed crossPoints with dist 1:
    // accCrossPointCost = {0, 0.5, 1.5, 2.5, 3}
    for (int cpIdx = 0; cpIdx < numCrossPoints(); ++cpIdx) {
        std::int64_t delta = 0;
        if ((cpIdx + 1) < numCrossPoints()) {
            delta = (crossPoints[cpIdx + 1].location - crossPoints[cpIdx].location) / 2;
        }
        accCrossPointDistCost[cpIdx + 1] = crossPoints[cpIdx].location - crossPoints[0].location + delta;
    }
}

std::int64_t MetalLayer::getCrossPointRangeDistCost(const utils::IntervalT<int> &crossPointRange) const {
    assert(crossPointRange.IsValid());
    return accCrossPointDistCost[crossPointRange.high + 1] - accCrossPointDistCost[crossPointRange.low];
}

std::int64_t MetalLayer::getCrossPointRangeDist(const utils::IntervalT<int> &crossPointRange) const {
    return crossPoints[crossPointRange.high].location - crossPoints[crossPointRange.low].location;
}

utils::PointT<std::int64_t> MetalLayer::getLoc(const GridPoint &grid) const {
    utils::PointT<std::int64_t> loc;
    loc[direction] = tracks[grid.trackIdx].location;
    loc[1 - direction] = crossPoints[grid.crossPointIdx].location;
    return loc;
}

std::pair<utils::PointT<std::int64_t>, utils::PointT<std::int64_t>> MetalLayer::getLoc(const GridEdge &edge) const {
    utils::PointT<std::int64_t> loc1 = getLoc(edge.u);
    utils::PointT<std::int64_t> loc2 = getLoc(edge.v);

    if (loc1.x == loc2.x) {
        if (loc1.y < loc2.y)
            return {loc1, loc2};
        else
            return {loc2, loc1};
    } else {
        if (loc1.x < loc2.x)
            return {loc1, loc2};
        else
            return {loc2, loc1};
    }
}

BoxOnLayer MetalLayer::getLoc(const GridBoxOnLayer &gridBox) const {
    BoxOnLayer box;
    box.layerIdx = gridBox.layerIdx;
    box[direction].Set(tracks[gridBox.trackRange.low].location, tracks[gridBox.trackRange.high].location);
    box[1 - direction].Set(crossPoints[gridBox.crossPointRange.low].location,
                           crossPoints[gridBox.crossPointRange.high].location);
    return box;
}

GridPoint MetalLayer::getUpper(const GridPoint &cur) const {
    return {cur.layerIdx + 1, crossPoints[cur.crossPointIdx].upperTrackIdx, tracks[cur.trackIdx].upperCPIdx};
}

GridPoint MetalLayer::getLower(const GridPoint &cur) const {
    return {cur.layerIdx - 1, crossPoints[cur.crossPointIdx].lowerTrackIdx, tracks[cur.trackIdx].lowerCPIdx};
}

bool MetalLayer::isValid(const GridPoint &gridPt) const {
    return gridPt.trackIdx >= 0 && gridPt.trackIdx < tracks.size() &&             // track
           gridPt.crossPointIdx >= 0 && gridPt.crossPointIdx < crossPoints.size();// cross point
}

bool MetalLayer::isValid(const GridBoxOnLayer &gridBox) const {
    return isTrackRangeValid(gridBox.trackRange) && isCrossPointRangeValid(gridBox.crossPointRange);
}

std::int64_t MetalLayer::getParaRunSpace(const std::int64_t width, const std::int64_t length) const {
    int iWidth = parallelWidth.size() - 1;// first smaller than or equal to
    while (iWidth > 0 && parallelWidth[iWidth] >= width) {
        --iWidth;
    }
    if (length == 0) return parallelWidthSpace[iWidth][0];// fast return
    int iLength = parallelLength.size() - 1;              // first smaller than or equal to
    while (iLength > 0 && parallelLength[iLength] >= length) {
        --iLength;
    }
    return parallelWidthSpace[iWidth][iLength];
}

std::int64_t MetalLayer::getParaRunSpace(const utils::BoxT<std::int64_t> &targetMetal, const std::int64_t length) const {
    return getParaRunSpace(std::min(targetMetal.width(), targetMetal.height()), length);
}

std::int64_t MetalLayer::getSpace(const utils::BoxT<std::int64_t> &targetMetal, int dir, AggrParaRunSpace aggr) const {
    const std::int64_t range = targetMetal[1 - dir].range();
    std::int64_t space = getEolSpace(range);
    if (!space) {
        // parallel run spacing
        std::int64_t length = 0;
        if (aggr == AggrParaRunSpace::LARGER_LENGTH && targetMetal[1 - dir].range() > 100 * pitch) {
            // hack: assume at least two-pitch parallel run length
            length = (pitch * 2 + width);
        }
        space = getParaRunSpace(targetMetal, length);
    }
    // do not know the width of neighbor, so paraRunSpaceForLargerWidth
    if (aggr == AggrParaRunSpace::LARGER_WIDTH && paraRunSpaceForLargerWidth > space) {
        space = paraRunSpaceForLargerWidth;
    }
    return space;
}

std::int64_t MetalLayer::getEolSpace(const std::int64_t width) const { return (width < maxEolWidth) ? maxEolSpace : 0; }

bool MetalLayer::isEolViolation(const std::int64_t space, const std::int64_t width, const std::int64_t within) const {
    return (space < maxEolSpace && width < maxEolWidth && within < maxEolWithin);
}

bool MetalLayer::isEolViolation(const utils::BoxT<std::int64_t> &lhs, const utils::BoxT<std::int64_t> &rhs) const {
    for (unsigned dim = 0; dim != 2; ++dim) {
        const std::int64_t space = utils::Dist(lhs[1 - dim], rhs[1 - dim]);
        const std::int64_t width = std::min(lhs[dim].range(), rhs[dim].range());
        const std::int64_t within = utils::Dist(lhs[dim], rhs[dim]);
        if (isEolViolation(space, width, within)) return true;
    }
    return false;
}

std::int64_t MetalLayer::getCornerSpace(const std::int64_t width) const {
    if (cornerExceptEol && width < cornerEolWidth) return 0;

    int iWidth = cornerWidth.size() - 1;// first smaller than or equal to
    while (iWidth > 0 && cornerWidth[iWidth] >= width) {
        --iWidth;
    }
    return cornerWidthSpace[iWidth];
}

std::int64_t MetalLayer::getCornerSpace(const utils::BoxT<std::int64_t> &targetMetal) const {
    return getCornerSpace(std::min(targetMetal.width(), targetMetal.height()));
}

void MetalLayer::initWireRange() {
    const std::int64_t eolSpace = getEolSpace(width);
    const std::int64_t wireEndPointSpace = eolSpace ? eolSpace + width : defaultSpace + width;// consider two half width
    wireRange.resize(numCrossPoints(), {0, 0});
    int i, j;
    for (int cpIdx = 0; cpIdx < numCrossPoints(); ++cpIdx) {
        i = 0;
        while (cpIdx + i >= 0 && crossPoints[cpIdx].location - crossPoints[cpIdx + i].location < wireEndPointSpace) {
            --i;
        }
        j = 0;
        while (cpIdx + j < numCrossPoints() &&
               crossPoints[cpIdx + j].location - crossPoints[cpIdx].location < wireEndPointSpace) {
            ++j;
        }
        wireRange[cpIdx] = {i + 1, j - 1};
    }
}

//ostream& MetalLayer::printBasics(ostream& os) const {
//    os << name << ": dir=" << getDimension(direction) << ", idx=" << idx;
//    os << ", tracks=(locs=" << firstTrackLoc() << "-" << lastTrackLoc() << ",pitch=" << pitch << ",#=" << numTracks()
//       << ")";
//    os << ", crossPts=(locs=" << firstCrossPointLoc() << "-" << lastCrossPointLoc() << ",#=" << numCrossPoints() << ")";
//    os << ", #grids=" << numGridPoints() << ")";
//    return os;
//}
//
//ostream& MetalLayer::printDesignRules(ostream& os) const {
//    os << name << ": width=" << width << ", paraSpace=(default=" << defaultSpace;
//    for (int i = 0; i < parallelWidth.size(); ++i) {
//        os << ", " << parallelWidth[i] << ":" << parallelWidthSpace[i];
//    }
//    os << "), eolSpace=(";
//    for (const SpaceRule& spaceRule : spaceRules) {
//        if (spaceRule.hasEol /* && !spaceRule.hasPar */) {
//            os << spaceRule.eolWidth << ':' << spaceRule.space << ", ";
//        }
//    }
//    os << "), minArea=" << minArea;
//    return os;
//}
//
//ostream& MetalLayer::printViaOccupancyLUT(ostream& os) const {
//    auto getMaxSize2d = [](const vector<vector<vector<bool>>>& LUT, size_t& xSize, size_t& ySize) {
//        xSize = 0;
//        ySize = 0;
//        for (const vector<vector<bool>>& cpLUT : LUT) {
//            if (cpLUT.size()) {
//                xSize = max(xSize, cpLUT.size());
//                ySize = max(ySize, cpLUT[0].size());
//            }
//        }
//    };
//    auto getMaxSize1d = [](const vector<utils::IntervalT<int>>& ranges, utils::IntervalT<int>& yRange) {
//      yRange = {0, 0};
//      for (const auto& rangeCP : ranges) {
//          yRange = yRange.UnionWith(rangeCP);
//      }
//    };
//    size_t xSize, ySize;
//    utils::IntervalT<int> yRange;
//    getMaxSize1d(wireRange, yRange);
//    os << name << ": wire(" << wireRange.size() << ',' << yRange << ')';
//    if (wireBotVia.size()) {
//        getMaxSize2d(wireBotVia[0], xSize, ySize);  // TODO: print default
//        os << ", wireBotVia(" << wireBotVia.size() << ',' << xSize << ',' << ySize << ')';
//    }
//    if (wireTopVia.size()) {
//        getMaxSize2d(wireTopVia[0], xSize, ySize);
//        os << ", wireTopVia(" << wireTopVia.size() << ',' << xSize << ',' << ySize << ')';
//    }
//    return os;
//}
//
//ostream& operator<<(ostream& os, const MetalLayer& layer) { return layer.printBasics(os); }

void MetalLayer::check() const {
    if (width < minWidth) {
        std::cout << "Warning: In layer " << name << ", width = " << width << " < minWidth = " << minWidth << std::endl;
    }
    if (width > maxEolWidth) {
        std::cout << "Warning: In layer " << name << ", width = " << width << " > maxEolWidth = " << maxEolWidth
                  << std::endl;
    }
    if (width + defaultSpace > pitch) {
        std::cout << "Warning: In layer " << name << ", width + defaultSpace =" << width + defaultSpace
                  << " > picth = " << pitch << std::endl;
    }
}
