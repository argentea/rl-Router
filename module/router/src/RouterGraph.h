#pragma once

#include "database/src/Database.h"

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

// Note: GridGraph will be across both GridGraphBuilder & MazeRoute
class RouterGraph {
public:
    // getters
    bool hasEdge(int u, EdgeDirection dir) const { return conn[u][dir] != -1; }
    int getEdgeEndPoint(int u, EdgeDirection dir) const { return conn[u][dir]; }
    db::CostT getEdgeCost(int u, EdgeDirection dir) const { return edgeCost[u][dir]; }
    db::CostT getVertexCost(int u) const { return vertexCost[u]; }
	//TODO
	db::CostT getCost(int u, int v);
	db::CostT getPotentialPenalty(int u, int v);
	vector<int> getNeighbor(int u);
	int numOfPins();
	//End TODO
    bool isMinAreaFixable(int u) const { return minAreaFixable[u]; }
    db::GridPoint& getGridPoint(int u) { return vertexToGridPoint[u]; }
    int getEdgeNum() const { return edgeCount; }
    int getNodeNum() const { return conn.size(); }
    int getPinIdx(int u) const;
    vector<int>& getVertices(int pinIdx) { return pinToVertex[pinIdx]; }
    bool isFakePin(int u) const { return fakePins.find(u) != fakePins.end(); }

    void writeDebugFile(const std::string& fn) const;

private:
    int edgeCount;

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
};
