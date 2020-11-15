#pragma once

#include "CutLayer.h"
#include "GeoPrimitive.h"
#include <functional>
#include <memory>

namespace database {

class GridSteiner : public GridPoint {
public:
    int pinIdx;// -1 stands for "not pin"
    bool fakePin;
    std::shared_ptr<GridSteiner> parent;
    std::vector<std::shared_ptr<GridSteiner>> children;
    std::unique_ptr<GridEdge> extWireSeg;// extended wire segment for fixing min area
    const ViaType *viaType = nullptr;

    explicit GridSteiner(const GridPoint &gridPoint, int pinIndex = -1, bool isFakePin = false)
        : GridPoint(gridPoint), pinIdx(pinIndex), fakePin(isFakePin) {}

    bool isRealPin() const { return pinIdx >= 0 && !fakePin; }

    // Set/reset parent
    static void setParent(const std::shared_ptr<GridSteiner> &childNode, const std::shared_ptr<GridSteiner> &parentNode);

    static void resetParent(const std::shared_ptr<GridSteiner> &node);

    // Traverse
    static void preOrder(const std::shared_ptr<GridSteiner> &node,
                         const std::function<void(std::shared_ptr<GridSteiner>)> &visit);

    static void postOrder(const std::shared_ptr<GridSteiner> &node,
                          const std::function<void(std::shared_ptr<GridSteiner>)> &visit);

    static void postOrderCopy(const std::shared_ptr<GridSteiner> &node,
                              const std::function<void(std::shared_ptr<GridSteiner>)> &visit);

    // Merge two same-layer edges (assume they are on the same track)
    static void mergeNodes(const std::shared_ptr<GridSteiner> &root);
};

}// namespace database
