#ifndef RL_ROUTER_METALLAYER_H
#define RL_ROUTER_METALLAYER_H
#include "database/src/GeoPrimitive.h"
#include "utils/enum.h"
#include "utils/geo.h"
#include <string>
#include <vector>

namespace database {

class Track {
public:
    explicit Track(std::int64_t loc, int lowerIdx = -1, int upperIdx = -1) : location(loc), lowerCPIdx(lowerIdx), upperCPIdx(upperIdx) {}
    std::int64_t location;
    int lowerCPIdx;
    int upperCPIdx;
};

// Cross points are projection of tracks from upper and lower layers
class CrossPoint {
public:
    explicit CrossPoint(std::int64_t loc, int lowerIdx = -1, int upperIdx = -1)
        : location(loc), lowerTrackIdx(lowerIdx), upperTrackIdx(upperIdx) {}
    std::int64_t location;
    int lowerTrackIdx;
    int upperTrackIdx;
};

class SpaceRule {
public:
    SpaceRule(const std::int64_t space, const std::int64_t eolWidth, const std::int64_t eolWithin)
        : space(space), hasEol(true), eolWidth(eolWidth), eolWithin(eolWithin) {}
    SpaceRule(const std::int64_t space, const std::int64_t eolWidth, const std::int64_t eolWithin, const std::int64_t parSpace, const std::int64_t parWithin)
        : space(space),
          hasEol(true),
          eolWidth(eolWidth),
          eolWithin(eolWithin),
          hasPar(true),
          parSpace(parSpace),
          parWithin(parWithin) {}

    std::int64_t space = 0;
    bool hasEol = false;
    std::int64_t eolWidth = 0;
    std::int64_t eolWithin = 0;
    bool hasPar = false;
    std::int64_t parSpace = 0;
    std::int64_t parWithin = 0;
};

enum class AggrParaRunSpace { DEFAULT,
                              LARGER_WIDTH,
                              LARGER_LENGTH };

class MetalLayer {
public:
    //    MetalLayer(Rsyn::PhysicalLayer rsynLayer, const vector<Rsyn::PhysicalTracks>& rsynTracks, const std::int64_t libstd::int64_t);

    // Basic infomation
    std::string name;
    Dimension direction;// direction of track dimension
    int idx;            // layerIdx (consistent with Rsyn::xxx::getRelativeIndex())

    // Track (1D)
    std::int64_t pitch = 0;
    std::vector<Track> tracks;
    int numTracks() const { return tracks.size(); }
    std::int64_t firstTrackLoc() const { return tracks.front().location; }
    std::int64_t lastTrackLoc() const { return tracks.back().location; }
    bool isTrackRangeValid(const utils::IntervalT<int> &trackRange) const;
    bool isTrackRangeWeaklyValid(const utils::IntervalT<int> &trackRange) const;
    utils::IntervalT<int> getUpperCrossPointRange(const utils::IntervalT<int> &trackRange) const;
    utils::IntervalT<int> getLowerCrossPointRange(const utils::IntervalT<int> &trackRange) const;
    // search by location (range) (result may be invalid/empty)
    utils::IntervalT<int> getSurroundingTrack(std::int64_t loc) const;
    utils::IntervalT<int> rangeSearchTrack(const utils::IntervalT<std::int64_t> &locRange, bool includeBound = true) const;

    // CrossPoint (1D)
    std::vector<CrossPoint> crossPoints;
    int numCrossPoints() const { return crossPoints.size(); }
    std::int64_t firstCrossPointLoc() const { return crossPoints.front().location; }
    std::int64_t lastCrossPointLoc() const { return crossPoints.back().location; }
    bool isCrossPointRangeValid(const utils::IntervalT<int> &crossPointRange) const;
    // base grid cost without congestion penalty
    // edge cost = accCrossPointCost[crossPointRange.high + 1] - accCrossPointCost[crossPointRange.low]
    // cost is directly posed on grids instead of edges (easier to cross layers & handle corners)
    std::vector<std::int64_t> accCrossPointDistCost;
    void initAccCrossPointDistCost();
    std::int64_t getCrossPointRangeDistCost(const utils::IntervalT<int> &crossPointRange) const;
    std::int64_t getCrossPointRangeDist(const utils::IntervalT<int> &crossPointRange) const;

    // GridPoint (2D) = Track (1D) x CrossPoint (1D)
    std::uint64_t numGridPoints() const { return tracks.size() * crossPoints.size(); }
    bool isValid(const GridPoint &gridPt) const;
    bool isValid(const GridBoxOnLayer &gridBox) const;
    utils::PointT<std::int64_t> getLoc(const GridPoint &grid) const;
    BoxOnLayer getLoc(const GridBoxOnLayer &gridBox) const;
    std::pair<utils::PointT<std::int64_t>, utils::PointT<std::int64_t>> getLoc(const GridEdge &edge) const;
    GridPoint getUpper(const GridPoint &cur) const;
    GridPoint getLower(const GridPoint &cur) const;

    // Design rules
    // width
    std::int64_t width = 0;
    std::int64_t minWidth = 0;
    std::int64_t widthForSuffOvlp = 0;
    std::int64_t shrinkForSuffOvlp = 0;
    // minArea
    std::int64_t minArea = 0;
    std::int64_t minLenRaw = 0;
    std::int64_t minLenOneVia = 0;
    std::int64_t minLenTwoVia = 0;
    std::int64_t viaOvlpDist = 0;
    std::int64_t viaLenEqLen = 0;
    std::int64_t viaWidthEqLen = 0;
    bool hasMinLenVio(std::int64_t len) const { return len < getMinLen(); }
    bool hasMinLenVioAcc(std::int64_t len) const { return len < getMinLenAcc(len); }
    std::int64_t getMinLen() const { return minLenRaw; }
    std::int64_t getMinLenAcc(std::int64_t len) const { return len < viaOvlpDist ? minLenOneVia : minLenTwoVia; }
    // parallel spacing
    std::vector<std::int64_t> parallelWidth{0};
    std::vector<std::int64_t> parallelLength{0};
    std::vector<std::vector<std::int64_t>> parallelWidthSpace{{0}};
    std::int64_t defaultSpace = 0;
    std::int64_t paraRunSpaceForLargerWidth = 0;
    std::int64_t getParaRunSpace(std::int64_t width, std::int64_t length = 0) const;
    std::int64_t getParaRunSpace(const utils::BoxT<std::int64_t> &targetMetal, std::int64_t length = 0) const;
    // eol spacing
    // TODO: handle multiple spaceRules
    std::vector<SpaceRule> spaceRules;
    std::int64_t maxEolSpace = 0;
    std::int64_t maxEolWidth = 0;
    std::int64_t maxEolWithin = 0;
    std::int64_t getEolSpace(std::int64_t width) const;
    bool isEolViolation(std::int64_t space, std::int64_t width, std::int64_t within) const;
    bool isEolViolation(const utils::BoxT<std::int64_t> &lhs, const utils::BoxT<std::int64_t> &rhs) const;
    // corner spacing
    bool cornerExceptEol = false;
    std::int64_t cornerEolWidth = 0;
    std::vector<std::int64_t> cornerWidth{0};
    std::vector<std::int64_t> cornerWidthSpace{0};
    bool hasCornerSpace() const { return cornerWidthSpace.size() > 1 || cornerWidthSpace[0]; }
    std::int64_t getCornerSpace(std::int64_t width) const;
    std::int64_t getCornerSpace(const utils::BoxT<std::int64_t> &targetMetal) const;

    // Translate design rule
    // either both parallel-run spacing or eol spacing
    std::int64_t getSpace(const utils::BoxT<std::int64_t> &targetMetal, int dir, AggrParaRunSpace aggr) const;
    // there is parallel-run spacing with negative length if the targetMetal is not eol dominated
    bool isEolDominated(const utils::BoxT<std::int64_t> &targetMetal) const {
        return std::max(targetMetal.x.range(), targetMetal.y.range()) < maxEolWidth;
    }
    // margin for multi-thread safe and others
    std::int64_t minAreaMargin = 0;
    std::int64_t confLutMargin = 0;
    std::int64_t fixedMetalQueryMargin = 0;
    std::int64_t mtSafeMargin = 0;

    // Via conflict lookup table (true means "available" / no conflict)
    // 1. wire-via conflict (viaTypeIdx, crossPointIdx, trackIdx, crossPointIdx)
    std::vector<std::vector<std::vector<std::vector<bool>>>> wireBotVia;
    std::vector<std::vector<std::vector<std::vector<bool>>>> wireTopVia;
    std::vector<std::vector<std::vector<bool>>> mergedWireBotVia;
    std::vector<std::vector<std::vector<bool>>> mergedWireTopVia;
    bool isWireViaMultiTrack = false;
    // 2. wire-wire conflict (crossPointIdx, crossPointIdx)
    std::vector<utils::IntervalT<int>> wireRange;
    void initWireRange();

    //    ostream &printBasics(ostream &os) const;
    //    ostream &printDesignRules(ostream &os) const;
    //    ostream &printViaOccupancyLUT(ostream &os) const;
    //    friend ostream &operator<<(ostream &os, const MetalLayer &layer);

private:
    void check() const;
};
}// namespace database

#endif//RL_ROUTER_METALLAYER_H
