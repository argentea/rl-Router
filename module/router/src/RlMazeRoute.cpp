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
        updateSol(std::make_shared<Solution>(vertex, graph));
    }
    std::unordered_set<int> visitedPin = {startPin};
    int nPinToConnect = localNet.numOfPins() - 1;

    while (nPinToConnect != 0) {
		printf("RL::pinLeft: %d\n", nPinToConnect);
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

            for (auto v : graph.getNeighbor(newSol)) {
			
                db::CostT newCost = newSol->cost + graph.getCost(newSol, v);
				
                if (newCost < vertexCostUBs[v]) {
                    updateSol(std::make_shared<Solution>(newCost, v, newSol));
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
 			updateSol(std::make_shared<Solution>(vertex));
        }

        visitedPin.insert(dstPinIdx);
        nPinToConnect--;
    }

    return true;
}

void MazeRoute::getResult() {
    std::unordered_map<int, std::shared_ptr<db::GridSteiner>> visited;

    // back track from pin to source
    for (unsigned p = 0; p < localNet.numOfPins(); p++) {
        std::unordered_map<int, std::shared_ptr<db::GridSteiner>> curVisited;
        auto cur = pinSols[p];
        std::shared_ptr<db::GridSteiner> prevS;
        while (cur) {
            auto it = visited.find(cur->vertex);
            if (it != visited.end()) {
                // graft to an existing node
                if (prevS) {
                    db::GridSteiner::setParent(prevS, it->second);
                }
                break;
            } else {
                // get curS
                auto curS = std::make_shared<db::GridSteiner>(
                    graph.getGridPoint(cur->vertex), graph.getPinIdx(cur->vertex), graph.isFakePin(cur->vertex));
                if (prevS) {
                    db::GridSteiner::setParent(prevS, curS);
                }
                if (curVisited.find(cur->vertex) != curVisited.end() && db::setting.singleNetVerbose >= +db::VerboseLevelT::MIDDLE) {
                    printlog("Warning: self loop found in a path for net", localNet.getName(), "for pin", p);
                }
                curVisited.emplace(cur->vertex, curS);
                // store tree root
                if (!(cur->prev)) {
                    localNet.gridTopo.push_back(curS);
                    break;
                }
                // prep for the next loop
                prevS = curS;
                cur = cur->prev;
            }
        }
        for (const auto &v : curVisited) visited.insert(v);
    }

    // remove redundant Steiner nodes
    for (auto &tree : localNet.gridTopo) {
        db::GridSteiner::mergeNodes(tree);
    }
}
