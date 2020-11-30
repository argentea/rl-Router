#pragma once
#include <cstdint>
#include <iostream>
#include <vector>

namespace router::db {

class Track {
public:
    explicit Track(int64_t loc, int lowerIdx = -1, int upperIdx = -1) : location(loc), lowerCPIdx(lowerIdx), upperCPIdx(upperIdx) {}
    int64_t location;
    int lowerCPIdx;
    int upperCPIdx;
};

// Cross points are projection of tracks from upper and lower layers
class CrossPoint {
public:
    explicit CrossPoint(int64_t loc, int lowerIdx = -1, int upperIdx = -1)
        : location(loc), lowerTrackIdx(lowerIdx), upperTrackIdx(upperIdx) {}
    int64_t location;
    int lowerTrackIdx;
    int upperTrackIdx;
};

class SpaceRule {
public:
    SpaceRule(const int64_t space, const int64_t eolWidth, const int64_t eolWithin)
        : space(space), hasEol(true), eolWidth(eolWidth), eolWithin(eolWithin) {}
    SpaceRule(const int64_t space, const int64_t eolWidth, const int64_t eolWithin, const int64_t parSpace, const int64_t parWithin)
        : space(space),
          hasEol(true),
          eolWidth(eolWidth),
          eolWithin(eolWithin),
          hasPar(true),
          parSpace(parSpace),
          parWithin(parWithin) {}

    int64_t space = 0;
    bool hasEol = false;
    int64_t eolWidth = 0;
    int64_t eolWithin = 0;
    bool hasPar = false;
    int64_t parSpace = 0;
    int64_t parWithin = 0;
};

enum class AggrParaRunSpace { DEFAULT,
                              LARGER_WIDTH,
                              LARGER_LENGTH };
enum Dimension {
    X = 0,
    Y = 1
};

// note: for operations on GeoPrimitives, all checking is down in LayerList for low level efficiency
class MetalLayer {
public:
    // Basic infomation
    std::string name;
    Dimension direction;// direction of track dimension
    int idx;            // layerIdx (consistent with Rsyn::xxx::getRelativeIndex())

    // Track (1D)
    int64_t pitch = 0;
    std::vector<Track> tracks;
    int numTracks() const { return tracks.size(); }
    int64_t firstTrackLoc() const { return tracks.front().location; }
    int64_t lastTrackLoc() const { return tracks.back().location; }

    // CrossPoint (1D)
    std::vector<CrossPoint> crossPoints;
    int numCrossPoints() const { return crossPoints.size(); }
    int64_t firstCrossPointLoc() const { return crossPoints.front().location; }
    int64_t lastCrossPointLoc() const { return crossPoints.back().location; }
    std::vector<int64_t> accCrossPointDistCost;

    // GridPoint (2D) = Track (1D) x CrossPoint (1D)
    int numGridPoints() const { return tracks.size() * crossPoints.size(); }

    // Design rules
    // width
    int64_t width = 0;
    int64_t minWidth = 0;
    int64_t widthForSuffOvlp = 0;
    int64_t shrinkForSuffOvlp = 0;
    // minArea
    int64_t minArea = 0;
    int64_t minLenRaw = 0;
    int64_t minLenOneVia = 0;
    int64_t minLenTwoVia = 0;
    int64_t viaOvlpDist = 0;
    int64_t viaLenEqLen = 0;
    int64_t viaWidthEqLen = 0;
    bool hasMinLenVio(int64_t len) const { return len < getMinLen(); }
    bool hasMinLenVioAcc(int64_t len) const { return len < getMinLenAcc(len); }
    int64_t getMinLen() const { return minLenRaw; }
    int64_t getMinLenAcc(int64_t len) const { return len < viaOvlpDist ? minLenOneVia : minLenTwoVia; }
    // parallel spacing
    std::vector<int64_t> parallelWidth{0};
    std::vector<int64_t> parallelLength{0};
    std::vector<std::vector<int64_t>> parallelWidthSpace{{0}};
    int64_t defaultSpace = 0;
    int64_t paraRunSpaceForLargerWidth = 0;
    //    int64_t getParaRunSpace(const utils::BoxT<int64_t>& targetMetal, const int64_t length = 0) const;
    // eol spacing
    // TODO: handle multiple spaceRules
    std::vector<SpaceRule> spaceRules;
    int64_t maxEolSpace = 0;
    int64_t maxEolWidth = 0;
    int64_t maxEolWithin = 0;
    // corner spacing
    bool cornerExceptEol = false;
    int64_t cornerEolWidth = 0;
    std::vector<int64_t> cornerWidth{0};
    std::vector<int64_t> cornerWidthSpace{0};
    bool hasCornerSpace() const { return cornerWidthSpace.size() > 1 || cornerWidthSpace[0]; }

    // margin for multi-thread safe and others
    int64_t minAreaMargin = 0;
    int64_t confLutMargin = 0;
    int64_t fixedMetalQueryMargin = 0;
    int64_t mtSafeMargin = 0;


private:
};

}// namespace router::db
