#pragma once

#include "database/src/Database.h"
#include <RouteGrid.h>
#include <Setting.h>

enum EdgeDirection { BACKWARD = 0, FORWARD = 1, UP = 2, DOWN = 3, LEFT = 4, RIGHT = 5 };

const vector<EdgeDirection> directions = {BACKWARD, FORWARD, UP, DOWN, LEFT, RIGHT};
const vector<EdgeDirection> oppDirections = {FORWARD, BACKWARD, DOWN, UP, RIGHT, LEFT};

bool switchLayer(EdgeDirection direction);
bool wrongWay(EdgeDirection direction);
EdgeDirection getOppDir(EdgeDirection direction);

class GridGraphBuilder;
class GridGraphBuilderBase;


class Vertex {
public:
	int layerId;
	bool isFakePin;
	
};
class RouterGraph;
class Solution {
public:
    db::CostT cost;
    DBU len;           // length on current track
    db::CostT costUB;  // cost upper bound
    int vertex;
    std::shared_ptr<Solution> prev;

    Solution(db::CostT c, DBU l, db::CostT ub, int v, const std::shared_ptr<Solution> &p)
        : cost(c), len(l), costUB(ub), vertex(v), prev(p) {}

	bool addVertex(int v);

    friend ostream &operator<<(ostream &os, const Solution &sol);
};
// Note: GridGraph will be across both GridGraphBuilder & MazeRoute
class RouterGraph {
public:
	//TODO
	db::Setting& setting;
	db::RrrIterSetting rrrIterSetting;
    int getPinIdx(int u) const;
	db::CostT getCost(std::shared_ptr<Solution> sol, int v);
	db::CostT getPotentialPenalty(std::shared_ptr<Solution> sol, int v);
	vector<int> getNeighbor(std::shared_ptr<Solution> sol);
	//End TODO
    vector<int>& getVertices(int pinIdx) { return pinToVertex[pinIdx]; }
    void writeDebugFile(const std::string& fn) const;
	RouterGraph(db::Setting& settingData, db::RrrIterSetting& rrrIterSettingData): setting{settingData}, rrrIterSetting{rrrIterSettingData} {}

private:
    int edgeCount;
//	db::Database& database;

    // vertex properties
    std::unordered_map<int, int> vertexToPin;  // vertexIdx to pinIdx
    vector<vector<int>> pinToVertex;
    std::unordered_set<int> fakePins;  // diff-layer access point
    vector<db::GridPoint> vertexToGridPoint;
    vector<bool> minAreaFixable;

    // adj lists
    vector<std::array<int, 6>> conn;
    vector<db::CostT> vertexCost;
    vector<std::array<db::CostT, 6>> edgeCost;

    // setters
    void init(int nNodes);
    void setVertexCost(int u, db::CostT w) { vertexCost[u] = w; }
    void addEdge(int u, int v, EdgeDirection dir, db::CostT w);

	// help functions
    // getters
    db::CostT getEdgeCost(int u, EdgeDirection dir) const { return edgeCost[u][dir]; }
    bool hasEdge(int u, EdgeDirection dir) const { return conn[u][dir] != -1; }
    int getEdgeEndPoint(int u, EdgeDirection dir) const { return conn[u][dir]; }
    db::CostT getVertexCost(int u) const { return vertexCost[u]; }
    bool isMinAreaFixable(int u) const { return minAreaFixable[u]; }
    db::GridPoint& getGridPoint(int u) { return vertexToGridPoint[u]; }
    int getEdgeNum() const { return edgeCount; }
    int getNodeNum() const { return conn.size(); }
    bool isFakePin(int u) const { return fakePins.find(u) != fakePins.end(); }

	friend class MazeRoute;
	friend class GridGraphBuilderBase;
	friend class GridGraphBuilder;
};
