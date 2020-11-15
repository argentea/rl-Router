#include "Net.h"
//#include "Database.h"

#include <fstream>

namespace database {

NetBase::~NetBase() {
    postOrderVisitGridTopo([](const std::shared_ptr<GridSteiner> &node) {
        node->children.clear();// free the memory
    });
}

BoxOnLayer NetBase::getMaxAccessBox(int pinIdx) const {
    std::int64_t maxArea = std::numeric_limits<std::int64_t>::min();
    BoxOnLayer bestBox;
    for (const auto &box : pinAccessBoxes[pinIdx]) {
        if (maxArea < box.area()) {
            maxArea = box.area();
            bestBox = box;
        }
    }
    return bestBox;
}

void NetBase::postOrderVisitGridTopo(const std::function<void(std::shared_ptr<GridSteiner>)> &visit) const {
    for (const std::shared_ptr<GridSteiner> &tree : gridTopo) {
        GridSteiner::postOrder(tree, visit);
    }
}

//void Net::clearPostRouteResult() {
//    defWireSegments.clear();
//}

void Net::clearResult() {
    gridTopo.clear();
    //    clearPostRouteResult();
}
void Net::stash() {
    routeGuideVios_copy = (routeGuideVios);
    routeGuideRTrees_copy = (routeGuideRTrees);
    gridTopo_copy = gridTopo;
}
void Net::reset() {
    routeGuideVios = (routeGuideVios_copy);
    routeGuideRTrees = (routeGuideRTrees_copy);
    gridTopo = gridTopo_copy;
}

}// namespace database
