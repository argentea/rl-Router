#pragma once
#include <algorithm>
#include <iostream>
#include <vector>

namespace router::db {

class GridPoint {
public:
    int layerIdx;
    int trackIdx;
    int crossPointIdx;

    explicit GridPoint(int layerIndex = -1, int trackIndex = -1, int crossPointIndex = -1)
        : layerIdx(layerIndex), trackIdx(trackIndex), crossPointIdx(crossPointIndex) {}

    bool operator==(const GridPoint &rhs) const;
    bool operator!=(const GridPoint &rhs) const;

    friend std::ostream &operator<<(std::ostream &os, const GridPoint &gp);
};

}// namespace router::db

namespace std {

// hash function for GridPoint
template<>
struct hash<router::db::GridPoint> {
    std::size_t operator()(const router::db::GridPoint &gp) const {
        return (std::hash<int>()(gp.layerIdx) ^ std::hash<int>()(gp.trackIdx) ^ std::hash<int>()(gp.crossPointIdx));
    }
};

}// namespace std

namespace router::db {

// GridEdge

class GridEdge {
public:
    GridPoint u, v;

    GridEdge(const GridPoint &nodeU, const GridPoint &nodeV) : u(nodeU), v(nodeV) {}

    // two types of GridEdge: 1. via, 2. track segment
    bool isVia() const;
    const GridPoint &lowerGridPoint() const { return u.layerIdx <= v.layerIdx ? u : v; }
    bool isTrackSegment() const;
    bool isWrongWaySegment() const;

    bool operator==(const GridEdge &rhs) const;

    friend std::ostream &operator<<(std::ostream &os, const GridEdge &edge);
};

class TrackSegment {
public:
    int layerIdx;
    int trackIdx;
    //    utils::IntervalT<int> crossPointRange;

    // assume edge.isTrackSegment()
    explicit TrackSegment(const GridEdge edge)
        : layerIdx(edge.u.layerIdx),
          trackIdx(edge.u.trackIdx) {}

    TrackSegment(int layerIndex, int trackIndex)
        : layerIdx(layerIndex), trackIdx(trackIndex) {}
};

class WrongWaySegment {
public:
    int layerIdx;
    int crossPointIdx;

    // assume edge.isTrackSegment()
    explicit WrongWaySegment(const GridEdge edge)
        : layerIdx(edge.u.layerIdx),
          crossPointIdx(edge.u.crossPointIdx) {
    }

    WrongWaySegment(int layerIndex /*, const utils::IntervalT<int>& trackIndexRange*/, int crossPointIndex)
        : layerIdx(layerIndex) /*, trackRange(trackIndexRange)*/, crossPointIdx(crossPointIndex) {}
};

// GridBoxOnLayer

class GridBoxOnLayer {
public:
    int layerIdx;

    GridBoxOnLayer() : layerIdx(-1) {}// default to be invalid

    GridBoxOnLayer(int layerIndex)
        : layerIdx(layerIndex) /*, trackRange(trackIdxRange), crossPointRange(crossPointIdxRange)*/ {}

    bool includePoint(const GridPoint &point) const {
        return layerIdx == point.layerIdx /* && trackRange.Contain(point.trackIdx) &&
               crossPointRange.Contain(point.crossPointIdx)*/
                ;
    }

    // slice polygons along sliceDir
    // sliceDir: 0 for x/vertical, 1 for y/horizontal
    // assume boxes are on the same layer
    static void sliceGridPolygons(std::vector<GridBoxOnLayer> &boxes);
};

// ViaBox

class ViaBox {
public:
    GridBoxOnLayer lower, upper;

    ViaBox() = default;// default to be invalid

    ViaBox(const GridBoxOnLayer &lowerGridBox, const GridBoxOnLayer &upperGridBox)
        : lower(lowerGridBox), upper(upperGridBox) {}
};

}// namespace router::db
