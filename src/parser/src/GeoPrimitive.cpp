#include "GeoPrimitive.h"

using namespace router::parser;
// BoxOnLayer

bool BoxOnLayer::isConnected(const BoxOnLayer &rhs) const {
    return abs(rhs.layerIdx - layerIdx) < 2 && HasIntersectWith(rhs);
}

// GridPoint

bool GridPoint::operator==(const GridPoint &rhs) const {
    return layerIdx == rhs.layerIdx && crossPointIdx == rhs.crossPointIdx && trackIdx == rhs.trackIdx;
}

bool GridPoint::operator!=(const GridPoint &rhs) const {
    return layerIdx != rhs.layerIdx || crossPointIdx != rhs.crossPointIdx || trackIdx != rhs.trackIdx;
}


bool GridEdge::isTrackSegment() const { return u.layerIdx == v.layerIdx && u.trackIdx == v.trackIdx; }

bool GridEdge::isWrongWaySegment() const { return u.layerIdx == v.layerIdx && u.crossPointIdx == v.crossPointIdx; }

bool GridEdge::operator==(const GridEdge &rhs) const { return u == rhs.u && v == rhs.v; }

// GridBoxOnLayer

bool GridBoxOnLayer::operator==(const GridBoxOnLayer &rhs) const {
    return layerIdx == rhs.layerIdx && trackRange == rhs.trackRange && crossPointRange == rhs.crossPointRange;
}

void GridBoxOnLayer::sliceGridPolygons(std::vector<GridBoxOnLayer> &boxes) {
    if (boxes.size() <= 1) return;

    std::vector<int> locs;
    for (const auto &box : boxes) {
        locs.push_back(box.trackRange.low);
        locs.push_back(box.trackRange.high);
    }
    sort(locs.begin(), locs.end());
    locs.erase(unique(locs.begin(), locs.end()), locs.end());

    // slice each box
    std::vector<GridBoxOnLayer> slicedBoxes;
    for (const auto &box : boxes) {
        GridBoxOnLayer slicedBox = box;
        auto itLoc = lower_bound(locs.begin(), locs.end(), box.trackRange.low);
        auto itEnd = upper_bound(itLoc, locs.end(), box.trackRange.high);
        slicedBox.trackRange.Set(*itLoc);
        slicedBoxes.push_back(slicedBox);// front boundary
        while ((itLoc + 1) != itEnd) {
            int left = *itLoc, right = *(itLoc + 1);
            if ((right - left) > 1) {
                slicedBox.trackRange.Set(left + 1, right - 1);
                slicedBoxes.push_back(slicedBox);// middle
            }
            slicedBox.trackRange.Set(right);
            slicedBoxes.push_back(slicedBox);// back boundary
            ++itLoc;
        }
    }
    boxes = move(slicedBoxes);

    // merge overlaped boxes over crossPoints
    MergeRects(boxes, 1);

    // stitch boxes over tracks
    MergeRects(boxes, 0);
}

