#ifndef RL_ROUTER_CUTLAYER_H
#define RL_ROUTER_CUTLAYER_H
#include "GeoPrimitive.h"

namespace router {
namespace parser {

class ViaType {
public:
    bool hasMultiCut = false;
    BoxT<int64_t> bot;// box on bottom metal layer
    BoxT<int64_t> top;// box on top metal layer
    BoxT<int64_t> cut;// box on cut layer
    std::string name;
    int idx;

    std::vector<BoxT<int64_t>> botForbidRegions;
    std::vector<BoxT<int64_t>> topForbidRegions;

    // via-wire conflict (crossPointIdx, trackIdx, crossPointIdx)
    std::vector<std::vector<std::vector<bool>>> viaBotWire;
    std::vector<std::vector<std::vector<bool>>> viaTopWire;

    // same-layer via-via conflict (viaTypeIdx, lowerTrackIdx, upperTrackIdx)
    // TODO: remove allViaMetal, rename allViaMetalNum to allViaMetal
    std::vector<std::vector<std::vector<bool>>> allViaCut;    // due to cut spacing
    std::vector<std::vector<std::vector<bool>>> allViaMetal;  // due to metal spacing
    std::vector<std::vector<std::vector<int>>> allViaMetalNum;// due to metal spacing, integer version

    // cross-layer via-via conflict (viaTypeIdx, crossPointIdx, trackIdx, crossPointIdx)
    std::vector<std::vector<std::vector<std::vector<bool>>>> allViaBotVia;
    std::vector<std::vector<std::vector<std::vector<bool>>>> allViaTopVia;

    // merged LUTs
    std::vector<std::vector<bool>> mergedAllViaMetal;
    std::vector<std::vector<std::vector<bool>>> mergedAllViaBotVia;
    std::vector<std::vector<std::vector<bool>>> mergedAllViaTopVia;

    ViaType() {}
    //    ViaType(Rsyn::PhysicalVia rsynVia);

    // alphabetical score tuple (belowWidth, aboveWidth, belowLength, aboveLength)
    std::tuple<int64_t, int64_t, int64_t, int64_t> getDefaultScore(int botDim, int topDim) const;

    // shifted bot/top metal
    BoxT<int64_t> getShiftedBotMetal(const PointT<int64_t> &viaPos) const;
    BoxT<int64_t> getShiftedTopMetal(const PointT<int64_t> &viaPos) const;
};

class CutLayer {
public:
    // Basic infomation
    std::string name;
    int idx;// layerIdx (consistent with Rsyn::xxx::getRelativeIndex())

    // Design rules
    int64_t width = 0;
    int64_t spacing = 0;

    // Via types
    std::vector<ViaType> allViaTypes;
    const ViaType &defaultViaType() const { return allViaTypes[0]; }
    bool isDefaultViaType(const ViaType &viaType) const { return viaType.idx == defaultViaType().idx; }
    BoxT<int64_t> topMaxForbidRegion;
    BoxT<int64_t> botMaxForbidRegion;

    // Via conflict lookup table (true means "not available" / with conflict)
    // 1. same-layer via-via conflict (lowerTrackIdx, upperTrackIdx)
    //  due to cut spacing
    const std::vector<std::vector<bool>> &viaCut() const { return defaultViaType().allViaCut[0]; }
    //  due to metal spacing
    const std::vector<std::vector<bool>> &viaMetal() const { return defaultViaType().allViaMetal[0]; }
    const std::vector<std::vector<int>> &viaMetalNum() const { return defaultViaType().allViaMetalNum[0]; }
    // 2. via-via conflict (crossPointIdx, trackIdx, crossPointIdx)
    const std::vector<std::vector<std::vector<bool>>> &viaBotVia() const { return defaultViaType().allViaBotVia[0]; }
    const std::vector<std::vector<std::vector<bool>>> &viaTopVia() const { return defaultViaType().allViaTopVia[0]; }
    // 3. via-wire conflict (crossPointIdx, trackIdx, crossPointIdx)
    const std::vector<std::vector<std::vector<bool>>> &viaBotWire() const { return defaultViaType().viaBotWire; }
    const std::vector<std::vector<std::vector<bool>>> &viaTopWire() const { return defaultViaType().viaTopWire; }

    //    ostream &printBasics(ostream &os) const;
    //    ostream &printDesignRules(ostream &os) const;
    //    ostream &printViaOccupancyLUT(ostream &os) const;
    //    friend ostream &operator<<(ostream &os, const CutLayer &layer);
};

}// namespace parser
}// namespace router
#endif