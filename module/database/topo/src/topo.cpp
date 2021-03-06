#include"topo.h"

namespace db {

// BoxOnLayer

bool BoxOnLayer::isConnected(const BoxOnLayer& rhs) const {
    return abs(rhs.layerIdx - layerIdx) < 2 && HasIntersectWith(rhs);
}

std::ostream& operator<<(std::ostream& os, const BoxOnLayer& box) {
    os << "box(l=" << box.layerIdx << ", x=" << box[0] << ", y=" << box[1] << ")";
    return os;
}

/*utils::BoxT<int> getBoxFromRsynBounds(const Bounds& bounds) {
    return {bounds.getLower().x, bounds.getLower().y, bounds.getUpper().x, bounds.getUpper().y};
}

utils::BoxT<int> getBoxFromRsynGeometries(const vector<Rsyn::PhysicalViaGeometry>& geos) {
    utils::BoxT<int> box;
    for (const Rsyn::PhysicalViaGeometry& geo : geos) {
        box = box.UnionWith(getBoxFromRsynBounds(geo.getBounds()));
    }
    return box;
}
*/
// GridPoint

bool GridPoint::operator==(const GridPoint& rhs) const {
    return layerIdx == rhs.layerIdx && crossPointIdx == rhs.crossPointIdx && trackIdx == rhs.trackIdx;
}

bool GridPoint::operator!=(const GridPoint& rhs) const {
    return layerIdx != rhs.layerIdx || crossPointIdx != rhs.crossPointIdx || trackIdx != rhs.trackIdx;
}

std::ostream& operator<<(std::ostream& os, const GridPoint& gp) {
    os << "gPt(l=" << gp.layerIdx << ", t=" << gp.trackIdx << ", c=" << gp.crossPointIdx << ")";
    return os;
}

// GridEdge

std::ostream& operator<<(std::ostream& os, const GridEdge& edge) {
    os << "gEdge(" << edge.u << " " << edge.v << ")";
    return os;
}
//todo 判断isVia需要database提供layer关系的信息. 考虑到database不止一个.这里留给未来处理

/*bool GridEdge::isVia() const {
    const auto& lower = (u.layerIdx <= v.layerIdx) ? u : v;
    const auto& upper = (u.layerIdx > v.layerIdx) ? u : v;
    if ((lower.layerIdx + 1) == database.getLayerNum()) {
        return false;
    }
    return database.getUpper(lower) == upper;
}
*/
bool GridEdge::isTrackSegment() const { return u.layerIdx == v.layerIdx && u.trackIdx == v.trackIdx; }

bool GridEdge::isWrongWaySegment() const { return u.layerIdx == v.layerIdx && u.crossPointIdx == v.crossPointIdx; }

bool GridEdge::operator==(const GridEdge& rhs) const { return u == rhs.u && v == rhs.v; }

// GridBoxOnLayer

bool GridBoxOnLayer::operator==(const GridBoxOnLayer& rhs) const {
    return layerIdx == rhs.layerIdx && trackRange == rhs.trackRange && crossPointRange == rhs.crossPointRange;
}

std::ostream& operator<<(std::ostream& os, const GridBoxOnLayer& gb) {
    os << "gBox(l=" << gb.layerIdx << ", t=" << gb.trackRange << ", c=" << gb.crossPointRange << ")";
    return os;
}

// slice polygons along sliceDir
// sliceDir: 0 for x/vertical, 1 for y/horizontal
void GridBoxOnLayer::sliceGridPolygons(std::vector<GridBoxOnLayer>& boxes) {
    if (boxes.size() <= 1) return;

    std::vector<int> locs;
    for (const auto& box : boxes) {
        locs.push_back(box.trackRange.low);
        locs.push_back(box.trackRange.high);
    }
    sort(locs.begin(), locs.end());
    locs.erase(unique(locs.begin(), locs.end()), locs.end());

    // slice each box
    std::vector<GridBoxOnLayer> slicedBoxes;
    for (const auto& box : boxes) {
        GridBoxOnLayer slicedBox = box;
        auto itLoc = lower_bound(locs.begin(), locs.end(), box.trackRange.low);
        auto itEnd = upper_bound(itLoc, locs.end(), box.trackRange.high);
        slicedBox.trackRange.Set(*itLoc);
        slicedBoxes.push_back(slicedBox);  // front boundary
        while ((itLoc + 1) != itEnd) {
            int left = *itLoc, right = *(itLoc + 1);
            if ((right - left) > 1) {
                slicedBox.trackRange.Set(left + 1, right - 1);
                slicedBoxes.push_back(slicedBox);  // middle
            }
            slicedBox.trackRange.Set(right);
            slicedBoxes.push_back(slicedBox);  // back boundary
            ++itLoc;
        }
    }
    boxes = move(slicedBoxes);

    // merge overlaped boxes over crossPoints
    utils::MergeRects(boxes, 1);

    // stitch boxes over tracks
    utils::MergeRects(boxes, 0);
}

// TrackSegment

std::ostream& operator<<(std::ostream& os, const TrackSegment& ts) {
    os << "tSeg(l=" << ts.layerIdx << ", t" << ts.trackIdx << ", c=" << ts.crossPointRange << ")";
    return os;
}

// WrongWaySegment

std::ostream& operator<<(std::ostream& os, const WrongWaySegment& wws) {
    os << "tSeg(l=" << wws.layerIdx << ", t" << wws.trackRange << ", c=" << wws.crossPointIdx << ")";
    return os;
}

// ViaBox

std::ostream& operator<<(std::ostream& os, const ViaBox& vb) {
    os << "viaBox(lower=" << vb.lower << ", upper=" << vb.upper << ")";
    return os;
}

}
