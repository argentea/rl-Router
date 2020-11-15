#ifndef RL_ROUTER_LAYERLIST_H
#define RL_ROUTER_LAYERLIST_H

#include "CutLayer.h"
#include "MetalLayer.h"

namespace database {

//class Database;

class LayerList {
public:
    //    void init(Database const &database, Rsyn::Session &session);

    // Check whether a geo primitive is valid
    bool isValid(const GridPoint &gridPt) const;
    bool isValid(const GridBoxOnLayer &gridBox) const;
    bool isValid(const ViaBox &viaBox) const;

    // Get metal rectangle (diff-net) forbidding region
    // 1. for pin taps
    BoxOnLayer getMetalRectForbidRegion(const BoxOnLayer &metalRect, AggrParaRunSpace aggr) const;
    // 2. for via forbid regions
    std::vector<utils::BoxT<std::int64_t>> getAccurateMetalRectForbidRegions(const BoxOnLayer &metalRect) const;

    // Expand box by several pitches
    void expandBox(BoxOnLayer &box, int numPitchToExtend) const;
    void expandBox(BoxOnLayer &box, int numPitchToExtend, int dir) const;

    // NOTE: the functions below assume valid input geo primitive(s)

    // Search by location
    utils::IntervalT<int> getSurroundingTrack(int layerIdx, std::int64_t loc) const;
    utils::IntervalT<int> getSurroundingCrossPoint(int layerIdx, std::int64_t loc) const;
    GridBoxOnLayer getSurroundingGrid(int layerIdx, utils::PointT<std::int64_t> loc) const;

    // Search by location range
    // input: layer (a valid one is assumed), location range [min, max] (inclusive)
    // output: index range of Track/CrossPoint
    // note: if out of range, assign the nearest endpoint
    utils::IntervalT<int> rangeSearchTrack(int layerIdx,
                                           const utils::IntervalT<std::int64_t> &locRange,
                                           bool includeBound = true) const;
    utils::IntervalT<int> rangeSearchCrossPoint(int layerIdx,
                                                const utils::IntervalT<std::int64_t> &locRange,
                                                bool includeBound = true) const;
    GridBoxOnLayer rangeSearch(const BoxOnLayer &box, bool includeBound = true) const;

    // Get (x, y) location of GridPoint/GridBoxOnLayer
    utils::PointT<std::int64_t> getLoc(const GridPoint &gridPt) const;
    BoxOnLayer getLoc(const GridBoxOnLayer &gridBox) const;
    std::pair<utils::PointT<std::int64_t>, utils::PointT<std::int64_t>> getLoc(const GridEdge &edge) const;

    // Find the upper/lower GridPoint/GridBoxOnLayer of the current GridPoint/GridBoxOnLayer
    // note: 1. valid layer is assumed, 2. upper/lower GridPoint/GridBoxOnLayer may not exist.
    GridPoint getUpper(const GridPoint &cur) const;
    GridPoint getLower(const GridPoint &cur) const;
    GridBoxOnLayer getUpper(const GridBoxOnLayer &cur) const;
    GridBoxOnLayer getLower(const GridBoxOnLayer &cur) const;

    // Get ViaBox from the intersection of two BoxOnLayer (in neighboring layers)
    // note: 1. should be on neighboring layers, 2. ViaBox may be empty.
    ViaBox getViaBoxBetween(const BoxOnLayer &lower, const BoxOnLayer &upper) const;
    ViaBox getViaBoxBetween(const GridBoxOnLayer &lower, const GridBoxOnLayer &upper) const {
        return getViaBoxBetween(getLoc(lower), getLoc(upper));
    }
    bool isConnected(const GridBoxOnLayer &lhs, const GridBoxOnLayer &rhs);
    bool isAdjacent(const GridBoxOnLayer &lhs, const GridBoxOnLayer &rhs);

    Dimension getLayerDir(int layerIdx) const { return layers[layerIdx].direction; }
    const MetalLayer &getLayer(int layerIdx) const { return layers[layerIdx]; }
    const CutLayer &getCutLayer(int cutLayerIdx) const { return cutLayers[cutLayerIdx]; }
    unsigned getLayerNum() const noexcept { return layers.size(); }

    // Merge LUTs
    void mergeLUT(std::vector<std::vector<bool>> &lhs, const std::vector<std::vector<bool>> &rhs);
    std::vector<std::vector<bool>> mergeLUTs(const std::vector<std::vector<std::vector<bool>>> &LUTs);
    std::vector<std::vector<std::vector<bool>>> mergeLUTsCP(const std::vector<std::vector<std::vector<std::vector<bool>>>> &LUTs);

protected:
    std::vector<MetalLayer> layers;
    std::vector<CutLayer> cutLayers;

    int numGridPoints;
    std::int64_t totalTrackLength;
    int numVias;

    void initCrossPoints();
    static void initOppLUT(const std::vector<std::vector<std::vector<bool>>> &ori, std::vector<std::vector<std::vector<bool>>> &opp);
    void initViaWire(int layerIdx, const utils::BoxT<std::int64_t> &viaMetal, std::vector<std::vector<std::vector<bool>>> &viaWireLUT);
    void initSameLayerViaConfLUT(int layerIdx,
                                 ViaType &viaT1,
                                 ViaType &viaT2,
                                 std::vector<std::vector<bool>> &viaCut,
                                 std::vector<std::vector<bool>> &viaMetal,
                                 std::vector<std::vector<int>> &viaMetalNum);
    void initDiffLayerViaConfLUT(int layerIdx,
                                 ViaType &viaT1,
                                 ViaType &viaT2,
                                 std::vector<std::vector<std::vector<bool>>> &viaBotVia,
                                 std::vector<std::vector<std::vector<bool>>> &viaTopVia);
    void initViaConfLUT();
    void initViaForbidRegions();
    //    void print();
    void writeDefConflictLUTs(const std::string &debugFileName) const;
};

}// namespace database
#endif