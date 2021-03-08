#include "RlMazeRoute.h"

bool MazeRoute::route(int startPin) {
    // define std::priority_queue
    auto solComp = [](const std::shared_ptr<Solution> &lhs, const std::shared_ptr<Solution> &rhs) {
        return rhs->cost < lhs->cost || (rhs->cost == lhs->cost && rhs->costUB < lhs->costUB);
    };
    std::priority_queue<std::shared_ptr<Solution>, vector<std::shared_ptr<Solution>>, decltype(solComp)> solQueue(
        solComp);

    auto updateSol = [&](const std::shared_ptr<Solution> &sol) {
        solQueue.push(sol);
        if (sol->costUB < vertexCostUBs[sol->vertex]) {
            vertexCostUBs[sol->vertex] = sol->costUB;
        }
    };

    // init from startPin
    for (auto vertex : graph.getVertices(startPin)) {
        updateSol(std::make_shared<Solution>(
            graph.getCost(startPin, vertex), graph.getCost(startPin, vertex), vertex, nullptr));
    }
    std::unordered_set<int> visitedPin = {startPin};
    int nPinToConnect = graph.numOfPins() - 1;

    while (nPinToConnect != 0) {
        std::shared_ptr<Solution> dstVertex;
        int dstPinIdx = -1;

        // Dijkstra
        while (!solQueue.empty()) {
            auto newSol = solQueue.top();
            int u = newSol->vertex;
            solQueue.pop();

            // reach a pin?
            dstPinIdx = graph.getPinIdx(u);
            if (dstPinIdx != -1 && visitedPin.find(dstPinIdx) == visitedPin.end()) {
                dstVertex = newSol;
                break;
            }

            // pruning by upper bound
            if (vertexCostUBs[u] < newSol->cost) continue;

            for (auto v : graph.getNeighbor(u)) {
			
                db::CostT newCost = newSol->cost + graph.getCost(u, v);
                db::CostT potentialPenalty = graph.getPotentialPenalty(u, v);
				
                if (newCost < vertexCostUBs[v]) {
                    updateSol(std::make_shared<Solution>(newCost, newCost + potentialPenalty, v, newSol));
                }
            }
        }

        if (!dstVertex) {
			return false;
        }

        // update pinSols
        pinSols[dstPinIdx] = dstVertex;

        // mark the path to be zero
        auto tmp = dstVertex;
        while (tmp && tmp->cost != 0) {
            updateSol(std::make_shared<Solution>(0, 0, tmp->vertex, tmp->prev));
            tmp = tmp->prev;
        }

        // mark all the accessbox of the pin to be almost zero
        for (auto vertex : graph.getVertices(dstPinIdx)) {
            if (vertex == dstVertex->vertex) continue;
            updateSol(std::make_shared<Solution>(
                graph.getVertexCost(vertex), graph.getVertexCost(vertex), vertex, nullptr));
        }

        visitedPin.insert(dstPinIdx);
        nPinToConnect--;
    }

    return true;
}

