#include "RouterGraph.h"
#include "database/grid/src/RouteGrid.h"
#include "database/layer/src/Layer.h"

#include <fstream>


Solution::Solution(int vertexdata, RouterGraph& graph) {
	len = graph.isFakePin(vertexdata) ? 0 : graph.database.getLayer(graph.getGridPoint(vertexdata).layerIdx).getMinLen();
	cost = graph.getVertexCost(vertexdata);
	costUB = graph.getVertexCost(vertexdata);
	vertex = vertexdata;
	prev = nullptr;
}

Solution::Solution(db::CostT newCost, int v, std::shared_ptr<Solution> &prevdata, RouterGraph& graph) {
	DBU newLen;            
	vertex = v;
	prev = prevdata;
	int u = prev->vertex;
	const db::MetalLayer &uLayer = graph.database.getLayer(graph.getGridPoint(u).layerIdx);
	const db::MetalLayer &vLayer = graph.database.getLayer(graph.getGridPoint(v).layerIdx);
	const db::GridPoint &vPoint = graph.getGridPoint(v);
	if (graph.getGridPoint(u).layerIdx == graph.getGridPoint(v).layerIdx) {
		const db::GridPoint &uPoint = graph.getGridPoint(u);
		newLen = prev->len;
		utils::IntervalT<int> cpRange =
			uPoint.crossPointIdx < vPoint.crossPointIdx
				? utils::IntervalT<int>(uPoint.crossPointIdx, vPoint.crossPointIdx)
				: utils::IntervalT<int>(vPoint.crossPointIdx, uPoint.crossPointIdx);
		utils::IntervalT<int> trackRange = uPoint.trackIdx < vPoint.trackIdx
											   ? utils::IntervalT<int>(uPoint.trackIdx, vPoint.trackIdx)
											   : utils::IntervalT<int>(vPoint.trackIdx, uPoint.trackIdx);
		newLen += uLayer.getCrossPointRangeDist(cpRange);
		newLen += uLayer.pitch * trackRange.range();
	} else {
		newLen = 0;
	}
	newLen = min(newLen, graph.database.getLayer(graph.getGridPoint(v).layerIdx).getMinLen());

	// potential minArea penalty
	db::CostT potentialPenalty = 0;
	if (vLayer.hasMinLenVioAcc(newLen)) {
		if (graph.isMinAreaFixable(v) || graph.getPinIdx(v) != -1) {
			potentialPenalty = vLayer.getMinLen() - newLen;
		} else {
			potentialPenalty = graph.database.getUnitMinAreaVioCost();
		}
	}
	len = newLen;
	costUB = newCost + potentialPenalty;

}

void RouterGraph::init(int nNodes) {
    conn.resize(nNodes, {-1, -1, -1, -1, -1, -1});
    edgeCost.resize(nNodes, {-1, -1, -1, -1, -1, -1});
    vertexCost.resize(nNodes, 0);

    edgeCount = 0;
}

void RouterGraph::addEdge(int u, int v, EdgeDirection dir, db::CostT w) {
    if (hasEdge(u, dir)) return;

    edgeCost[u][dir] = edgeCost[v][getOppDir(dir)] = w;
    conn[u][dir] = v;
    conn[v][oppDirections[dir]] = u;

    edgeCount++;
}

int RouterGraph::getPinIdx(int u) const {
    auto it = vertexToPin.find(u);
    return (it != vertexToPin.end()) ? it->second : -1;
}

db::CostT RouterGraph::getCost(std::shared_ptr<Solution> sol, int v){
	int u = sol->vertex;
    const db::MetalLayer &uLayer = database.getLayer(getGridPoint(u).layerIdx);
	const db::MetalLayer &vLayer = database.getLayer(getGridPoint(v).layerIdx);
	EdgeDirection direction;
	for (auto dir : directions) {
		if(v == getEdgeEndPoint(u, dir)) {
			direction = dir;
			break;
		}
	}
    bool areOverlappedVertexes = (switchLayer(direction) && getEdgeCost(u, direction) == 0);
    db::CostT w = areOverlappedVertexes ? 0 : getEdgeCost(u, direction) + getVertexCost(v);
	db::CostT penalty = 0;
    if (!areOverlappedVertexes && switchLayer(direction)) {
        if (uLayer.hasMinLenVioAcc(sol->len)) {
        	if (isMinAreaFixable(u) || getPinIdx(u) != -1) {
            	penalty = uLayer.getMinLen() - sol->len;
            } else {
            	penalty = database.getUnitMinAreaVioCost();
				printf("RL:: minArea vio cost%lf\n", database.getUnitMinAreaVioCost());
            }
        }
   	}
	return w + penalty;
}

db::CostT RouterGraph::getPotentialPenalty(std::shared_ptr<Solution> sol, int v){
	DBU newLen;
	int u = sol->vertex;
    const db::MetalLayer &uLayer = database.getLayer(getGridPoint(u).layerIdx);
	const db::MetalLayer &vLayer = database.getLayer(getGridPoint(v).layerIdx);
    const db::GridPoint &vPoint = getGridPoint(v);
    if (getGridPoint(u).layerIdx == getGridPoint(v).layerIdx) {
    	const db::GridPoint &uPoint = getGridPoint(u);
        newLen = sol->len;
        utils::IntervalT<int> cpRange =
            uPoint.crossPointIdx < vPoint.crossPointIdx
                ? utils::IntervalT<int>(uPoint.crossPointIdx, vPoint.crossPointIdx)
                : utils::IntervalT<int>(vPoint.crossPointIdx, uPoint.crossPointIdx);
        utils::IntervalT<int> trackRange = uPoint.trackIdx < vPoint.trackIdx
                                               ? utils::IntervalT<int>(uPoint.trackIdx, vPoint.trackIdx)
                                               : utils::IntervalT<int>(vPoint.trackIdx, uPoint.trackIdx);
        newLen += uLayer.getCrossPointRangeDist(cpRange);
        newLen += uLayer.pitch * trackRange.range();
    } else {
        newLen = 0;
    }
    newLen = min(newLen, database.getLayer(getGridPoint(v).layerIdx).getMinLen());

    // potential minArea penalty
    db::CostT potentialPenalty = 0;
    if (vLayer.hasMinLenVioAcc(newLen)) {
        if (isMinAreaFixable(v) || getPinIdx(v) != -1) {
            potentialPenalty = vLayer.getMinLen() - newLen;
        } else {
            potentialPenalty = database.getUnitMinAreaVioCost();
        }
    }
	return potentialPenalty;
}

vector<int> RouterGraph::getNeighbor(std::shared_ptr<Solution> sol){
	vector<int> neighbor;
	int u = sol->vertex;
    for (auto direction : directions) {
        if (!hasEdge(u, direction) ||
            (sol->prev && getEdgeEndPoint(u, direction) == sol->prev->vertex)) {
            continue;
        }

		neighbor.push_back(getEdgeEndPoint(u, direction));
	}
	return neighbor;
}

/*
void GridGraph::writeDebugFile(const std::string& fn) const {
    std::ofstream debugFile(fn);
    for (int i = 0; i < conn.size(); ++i) {
        debugFile << vertexToGridPoint[i] << " vertexC=" << getVertexCost(i) << " edgeC=" << edgeCost[i] << std::endl;
    }
}
*/
bool switchLayer(EdgeDirection direction) { return direction == UP || direction == DOWN; }
bool wrongWay(EdgeDirection direction) { return direction == LEFT || direction == RIGHT; }
EdgeDirection getOppDir(EdgeDirection direction) { return oppDirections[direction]; }
