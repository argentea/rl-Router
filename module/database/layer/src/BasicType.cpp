#include "BasicType.h"

namespace router::db {

bool GridPoint::operator==(const GridPoint &rhs) const {
    return layerIdx == rhs.layerIdx && crossPointIdx == rhs.crossPointIdx && trackIdx == rhs.trackIdx;
}

bool GridPoint::operator!=(const GridPoint &rhs) const {
    return layerIdx != rhs.layerIdx || crossPointIdx != rhs.crossPointIdx || trackIdx != rhs.trackIdx;
}

std::ostream &operator<<(std::ostream &os, const GridPoint &gp) {
    os << "gPt(l=" << gp.layerIdx << ", t=" << gp.trackIdx << ", c=" << gp.crossPointIdx << ")";
    return os;
}

// GridEdge
std::ostream &operator<<(std::ostream &os, const GridEdge &edge) {
    os << "gEdge(" << edge.u << " " << edge.v << ")";
    return os;
}

bool GridEdge::isVia() const {
    const auto &lower = (u.layerIdx <= v.layerIdx) ? u : v;
    const auto &upper = (u.layerIdx > v.layerIdx) ? u : v;
    return true;
}

bool GridEdge::isTrackSegment() const { return u.layerIdx == v.layerIdx && u.trackIdx == v.trackIdx; }

bool GridEdge::isWrongWaySegment() const { return u.layerIdx == v.layerIdx && u.crossPointIdx == v.crossPointIdx; }

bool GridEdge::operator==(const GridEdge &rhs) const { return u == rhs.u && v == rhs.v; }

// slice polygons along sliceDir
// sliceDir: 0 for x/vertical, 1 for y/horizontal
void GridBoxOnLayer::sliceGridPolygons(std::vector<GridBoxOnLayer> &boxes) {
    if (boxes.size() <= 1) return;

    std::vector<int> locs;
    for (const auto &box : boxes) {
        //TODO: unfinished?
    }
    std::sort(locs.begin(), locs.end());
    locs.erase(std::unique(locs.begin(), locs.end()), locs.end());

    // slice each box
    std::vector<GridBoxOnLayer> slicedBoxes;
    for (const auto &box : boxes) {
        GridBoxOnLayer slicedBox = box;
        slicedBoxes.push_back(slicedBox);// front boundary
    }
    boxes = move(slicedBoxes);
}

}// namespace router::db
