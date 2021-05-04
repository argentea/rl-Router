#ifndef RL_ROUTER_CUTLAYER_H
#define RL_ROUTER_CUTLAYER_H
#include "GeoPrimitive.h"

namespace router::parser {

class ViaType {
public:
    bool hasMultiCut = false;
    utils::BoxT<DBU> bot;// box on bottom metal layer
    utils::BoxT<DBU> top;// box on top metal layer
    utils::BoxT<DBU> cut;// box on cut layer
    std::string name;
    int idx;

    // alphabetical score tuple (belowWidth, aboveWidth, belowLength, aboveLength)
    std::tuple<DBU, DBU, DBU, DBU> getDefaultScore(int botDim, int topDim) const {
        return {bot[botDim].range(),     // belowWidth
                top[topDim].range(),     // aboveWidth
                bot[1 - botDim].range(), // belowWidth
                top[1 - topDim].range()};// aboveLength
    }
};

class CutLayer {
public:
    // Basic information
    std::string name;
    int idx;// layerIdx (consistent with Rsyn::xxx::getRelativeIndex())

    // Design rules
    DBU width = 0;
    DBU spacing = 0;

    // Via types
    std::vector<ViaType> allViaTypes;
};

}// namespace router::parser
#endif
