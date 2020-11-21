#ifndef RL_ROUTER_METALLAYER_H
#define RL_ROUTER_METALLAYER_H

#include "GeoPrimitive.h"

namespace router {
namespace parser {

enum Direction {
    X = 0,
    Y = 1
};

class Track {
public:
    explicit Track(int64_t loc) : location(loc) {}
    int64_t location;
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


// note: for operations on GeoPrimitives, all checking is down in LayerList for low level efficiency
class MetalLayer {
public:
    // Basic infomation
    std::string name;
    Direction direction;// direction of track dimension
    int idx;      // layerIdx (consistent with Rsyn::xxx::getRelativeIndex())

    // Track (1D)
    int64_t pitch = 0;
    std::vector<Track> tracks;

    // Design rules
    // width
    int64_t width = 0;
    int64_t minWidth = 0;
    int64_t widthForSuffOvlp = 0;
    int64_t shrinkForSuffOvlp = 0;
    // minArea
    int64_t minArea = 0;
    int64_t minLenRaw = 0;
    // parallel spacing
    std::vector<int64_t> parallelWidth{0};
    std::vector<int64_t> parallelLength{0};
    std::vector<std::vector<int64_t>> parallelWidthSpace{{0}};
    int64_t defaultSpace = 0;
    int64_t paraRunSpaceForLargerWidth = 0;
    int64_t getParaRunSpace(int64_t width, int64_t length = 0) const;
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

    // Translate design rule
    // either both parallel-run spacing or eol spacing
    // there is parallel-run spacing with negative length if the targetMetal is not eol dominated
    // margin for multi-thread safe and others
    int64_t confLutMargin = 0;
    int64_t fixedMetalQueryMargin = 0;
};

}// namespace parser
}// namespace router

#endif