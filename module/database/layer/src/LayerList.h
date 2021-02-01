#pragma once
#include "Layer.h"
#include "parser/src/Parser.h"
#include <vector>

namespace db {

class LayerList {
public:
	void init(router::parser::Parser& parser);
    // Check whether a geo primitive is valid
    bool isValid(const GridPoint& gridPt) const;
    bool isValid(const GridBoxOnLayer& gridBox) const;
    bool isValid(const ViaBox& viaBox) const;

    // Get metal rectangle (diff-net) forbidding region
    // 1. for pin taps
    BoxOnLayer getMetalRectForbidRegion(const BoxOnLayer& metalRect, AggrParaRunSpace aggr) const;
    // 2. for via forbid regions
    vector<utils::BoxT<DBU>> getAccurateMetalRectForbidRegions(const BoxOnLayer& metalRect) const;

    // Expand box by several pitches
    void expandBox(BoxOnLayer& box, int numPitchToExtend) const;
    void expandBox(BoxOnLayer& box, int numPitchToExtend, int dir) const;

    // NOTE: the functions below assume valid input geo primitive(s)

    // Search by location
    utils::IntervalT<int> getSurroundingTrack(int layerIdx, DBU loc) const;
    utils::IntervalT<int> getSurroundingCrossPoint(int layerIdx, DBU loc) const;
    GridBoxOnLayer getSurroundingGrid(int layerIdx, utils::PointT<DBU> loc) const;



    Dimension getLayerDir(int layerIdx) const { return _layers[layerIdx].direction; }

    const MetalLayer &getLayer(int layerIdx) const { return _layers[layerIdx]; }

    //    const CutLayer& getCutLayer(int cutLayerIdx) const { return cutLayers[cutLayerIdx]; }
    unsigned getLayerNum() const noexcept { return _layers.size(); }
    utils::IntervalT<int> rangeSearchTrack(int layerIdx,
                                           const utils::IntervalT<DBU>& locRange,
                                           bool includeBound = true) const;
    utils::IntervalT<int> rangeSearchCrossPoint(int layerIdx,
                                                const utils::IntervalT<DBU>& locRange,
                                                bool includeBound = true) const;
    GridBoxOnLayer rangeSearch(const BoxOnLayer& box, bool includeBound = true) const;
    utils::PointT<DBU> getLoc(const GridPoint& gridPt) const;
    BoxOnLayer getLoc(const GridBoxOnLayer& gridBox) const;
    std::pair<utils::PointT<DBU>, utils::PointT<DBU>> getLoc(const GridEdge& edge) const;

    // Find the upper/lower GridPoint/GridBoxOnLayer of the current GridPoint/GridBoxOnLayer
    // note: 1. valid layer is assumed, 2. upper/lower GridPoint/GridBoxOnLayer may not exist.
    GridPoint getUpper(const GridPoint& cur) const;
    GridPoint getLower(const GridPoint& cur) const;
    GridBoxOnLayer getUpper(const GridBoxOnLayer& cur) const;
    GridBoxOnLayer getLower(const GridBoxOnLayer& cur) const;

    // Get ViaBox from the intersection of two BoxOnLayer (in neighboring layers)
    // note: 1. should be on neighboring layers, 2. ViaBox may be empty.
    ViaBox getViaBoxBetween(const BoxOnLayer& lower, const BoxOnLayer& upper);
    ViaBox getViaBoxBetween(const GridBoxOnLayer& lower, const GridBoxOnLayer& upper) {
        return getViaBoxBetween(getLoc(lower), getLoc(upper));
    }


protected:
    std::vector<MetalLayer> _layers;
	std::vector<MetalLayer> _cut_layers;

    int numGridPoints;
    int64_t totalTrackLength;
    int numVias;
};

}// namespace router::db
