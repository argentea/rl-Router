#ifndef RL_ROUTER_GEOPRIMITIVE_H
#define RL_ROUTER_GEOPRIMITIVE_H
#include "functional"
#include "geo.h"


namespace router {
namespace parser {
enum Dimension {
    X = 0,
    Y = 1
};// end enum
//class Database;

//  BoxOnLayer
//  A box on a certain layer: primitive for route guide and pin acesss box

class BoxOnLayer : public BoxT<int64_t> {
public:
    int layerIdx{0};

    //  constructors
    template<typename... Args>
    explicit BoxOnLayer(int layerIndex = -1, Args... params) : layerIdx(layerIndex), BoxT<int64_t>(params...) {}

    // inherit setters from BoxT in batch
    template<typename... Args>
    void Set(int layerIndex = -1, Args... params) {
        layerIdx = layerIndex;
        BoxT<int64_t>::Set(params...);
    }

    bool isConnected(const BoxOnLayer &rhs) const;
};

// helper
//BoxT<int64_t> getBoxFromRsynBounds(const Bounds &bounds);
//BoxT<int64_t> getBoxFromRsynGeometries(const vector<Rsyn::PhysicalViaGeometry> &geos);

// GridPoint

class GridPoint {
public:
    int layerIdx;
    int trackIdx;
    int crossPointIdx;

    explicit GridPoint(int layerIndex = -1, int trackIndex = -1, int crossPointIndex = -1)
        : layerIdx(layerIndex), trackIdx(trackIndex), crossPointIdx(crossPointIndex) {}

    bool operator==(const GridPoint &rhs) const;
    bool operator!=(const GridPoint &rhs) const;

    //    friend ostream &operator<<(ostream &os, const GridPoint &gp);
};

//}// namespace parser

//namespace std {
//
//// hash function for GridPoint
//template<>
//struct hash<parser::GridPoint> {
//    std::size_t operator()(const parser::GridPoint &gp) const {
//        return (std::hash<int>()(gp.layerIdx) ^ std::hash<int>()(gp.trackIdx) ^ std::hash<int>()(gp.crossPointIdx));
//    }
//};
//
//}// namespace std

//namespace parser {

// GridEdge

class GridEdge {
public:
    GridPoint u, v;

    GridEdge(const GridPoint &nodeU, const GridPoint &nodeV) : u(nodeU), v(nodeV) {}

    // two types of GridEdge: 1. via, 2. track segment
    //    bool isVia(Database const &database) const;
    const GridPoint &lowerGridPoint() const { return u.layerIdx <= v.layerIdx ? u : v; }
    bool isTrackSegment() const;
    bool isWrongWaySegment() const;

    bool operator==(const GridEdge &rhs) const;

    //    friend ostream &operator<<(ostream &os, const GridEdge &edge);
};

class TrackSegment {
public:
    int layerIdx;
    int trackIdx;
    IntervalT<int> crossPointRange;

    // assume edge.isTrackSegment()
    TrackSegment(const GridEdge edge)
        : layerIdx(edge.u.layerIdx),
          trackIdx(edge.u.trackIdx),
          crossPointRange(edge.u.crossPointIdx, edge.v.crossPointIdx) {
        if (!crossPointRange.IsValid()) std::swap(crossPointRange.low, crossPointRange.high);
    }

    TrackSegment(int layerIndex, int trackIndex, const IntervalT<int> &crossPointIndexRange)
        : layerIdx(layerIndex), trackIdx(trackIndex), crossPointRange(crossPointIndexRange) {}

    //    friend ostream &operator<<(ostream &os, const TrackSegment &ts);
};

class WrongWaySegment {
public:
    int layerIdx;
    IntervalT<int> trackRange;
    int crossPointIdx;

    // assume edge.isTrackSegment()
    WrongWaySegment(const GridEdge edge)
        : layerIdx(edge.u.layerIdx),
          trackRange(edge.u.trackIdx, edge.v.trackIdx),
          crossPointIdx(edge.u.crossPointIdx) {
        if (!trackRange.IsValid()) std::swap(trackRange.low, trackRange.high);
    }

    WrongWaySegment(int layerIndex, const IntervalT<int> &trackIndexRange, int crossPointIndex)
        : layerIdx(layerIndex), trackRange(trackIndexRange), crossPointIdx(crossPointIndex) {}

    //    friend ostream &operator<<(ostream &os, const WrongWaySegment &wws);
};

// GridBoxOnLayer

class GridBoxOnLayer {
public:
    int layerIdx;
    IntervalT<int> trackRange;
    IntervalT<int> crossPointRange;

    GridBoxOnLayer() : layerIdx(-1) {}// default to be invalid

    GridBoxOnLayer(int layerIndex,
                   const IntervalT<int> &trackIdxRange,
                   const IntervalT<int> &crossPointIdxRange)
        : layerIdx(layerIndex), trackRange(trackIdxRange), crossPointRange(crossPointIdxRange) {}

    bool includePoint(const GridPoint &point) const {
        return layerIdx == point.layerIdx && trackRange.Contain(point.trackIdx) &&
               crossPointRange.Contain(point.crossPointIdx);
    }

    // slice polygons along sliceDir
    // sliceDir: 0 for x/vertical, 1 for y/horizontal
    // assume boxes are on the same layer
    static void sliceGridPolygons(std::vector<GridBoxOnLayer> &boxes);
    const IntervalT<int> &operator[](unsigned i) const {
        assert(i == 0 || i == 1);
        return (i == 0) ? trackRange : crossPointRange;
    }
    IntervalT<int> &operator[](unsigned i) {
        assert(i == 0 || i == 1);
        return (i == 0) ? trackRange : crossPointRange;
    }

    bool operator==(const GridBoxOnLayer &rhs) const;

    //    friend ostream &operator<<(ostream &os, const GridBoxOnLayer &gb);
};

// ViaBox

class ViaBox {
public:
    GridBoxOnLayer lower, upper;

    ViaBox() = default;// default to be invalid

    ViaBox(const GridBoxOnLayer &lowerGridBox, const GridBoxOnLayer &upperGridBox)
        : lower(lowerGridBox), upper(upperGridBox) {}

    //    friend ostream &operator<<(ostream &os, const ViaBox &vb);
};

}// namespace parser
}// namespace router
#endif