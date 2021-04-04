#include "net.h"
#include "parser/src/Database.h"
#include "parser/src/Net.h"
#include "parser/src/Parser.h"

#include <fstream>


namespace db {
    
NetBase::~NetBase() {
//    postOrderVisitGridTopo([](std::shared_ptr<GridSteiner> node){
//        node->children.clear(); // free the memory
//    });
}

BoxOnLayer NetBase::getMaxAccessBox(int pinIdx) const {
    DBU maxArea = std::numeric_limits<DBU>::min();
    db::BoxOnLayer bestBox;
    for (const auto &box : pinAccessBoxes[pinIdx]) {
        if (maxArea < box.area()) {
            maxArea = box.area();
            bestBox = box;
        }
    }
    return bestBox;
}
/*
void NetBase::postOrderVisitGridTopo(const std::function<void(std::shared_ptr<GridSteiner>)>& visit) const {
    for (const std::shared_ptr<GridSteiner>& tree : gridTopo) {
        GridSteiner::postOrder(tree, visit);
    }
}
*/
void NetBase::printBasics(ostream& os) const {
    os << "Net " << getName() << " (idx = " << idx << ") with " << numOfPins() << " pins " << std::endl;
    os << routeGuides.size() << " route guides" << std::endl;
    if (routeGuides.size() == gridRouteGuides.size()) {
        for (unsigned int i = 0; i < routeGuides.size(); ++i) {
            os << routeGuides[i] << " " << gridRouteGuides[i] << std::endl;
        }
    }
    else {
        for (auto& routeGuide : routeGuides) {
            os << routeGuide << std::endl;
        }
    }

    os << std::endl;
}
/*
void NetBase::printResult(ostream& os) const {
    os << "grid topo: " << std::endl;
    for (const auto& tree : gridTopo) {
        tree->printTree(os);
        os << std::endl;
    }
    os << "extend seg: " << std::endl;
    postOrderVisitGridTopo([&](std::shared_ptr<GridSteiner> node) {
        if (node->extWireSeg) {
            os << *(node->extWireSeg) << " ";
        }
    });
    os << std::endl;
}
*/

Net::Net(int i, ParserNet net) {
    idx = i;
    parser_net = net;
    // pins
	pinAccessBoxes.reserve(net._pin_access_boxes.size());
	for(unsigned int i = 0; i < net._pin_access_boxes.size(); i++){
		pinAccessBoxes[i].reserve(net._pin_access_boxes[i].size());
		for(unsigned int j = 0; j < net._pin_access_boxes[i].size(); j++){
			pinAccessBoxes[i][j] = net._pin_access_boxes[i][j];
		}
	}

    // route guides
	for(const auto guide: net._route_guides){
		routeGuides.emplace_back(guide);
	}
    routeGuideVios.resize(routeGuides.size(), 0);
}

void Net::clearPostRouteResult() {
    defWireSegments.clear();
}

void Net::clearResult() {
//    gridTopo.clear();
    clearPostRouteResult();
}

void Net::stash() {
    routeGuideVios_copy = routeGuideVios;
//    routeGuideRTrees_copy = routeGuideRTrees;
//    gridTopo_copy = gridTopo;
}

void Net::reset() {
    routeGuideVios = (routeGuideVios_copy);
//    routeGuideRTrees = (routeGuideRTrees_copy);
//    gridTopo = gridTopo_copy;
}




void NetList::init(Database db) {

    nets.clear();
    nets.reserve(db._nets.size());
    int numPins = 0;
    for (auto db_net : db._nets) {
        nets.emplace_back(nets.size(), db_net);
        numPins += nets.back().pinAccessBoxes.size();
    }
}
/*
void NetList::writeNetTopo(const std::string& filename) {
    log() << "Write net topologies to " << filename << " ..." << std::endl;
    log() << std::endl;
    std::ofstream ofs(filename);

    for (const auto& net : nets) {
        ofs << net.getName() << " (idx = " << net.idx << ")" << std::endl;
        net.printResult(ofs);
    }
}
*/
}  // namespace db
