#include "GridTopo.h"

using namespace router::parser;

void GridSteiner::setParent(const std::shared_ptr<GridSteiner> &childNode, const std::shared_ptr<GridSteiner> &parentNode) {
    parentNode->children.push_back(childNode);
    childNode->parent = parentNode;
}

void GridSteiner::resetParent(const std::shared_ptr<GridSteiner> &node) {
    assert(node->parent);

    auto &n = node->parent->children;
    auto it = find(n.begin(), n.end(), node);
    assert(it != n.end());
    *it = n.back();
    n.pop_back();

    node->parent.reset();
}

void GridSteiner::preOrder(const std::shared_ptr<GridSteiner> &node, const std::function<void(std::shared_ptr<GridSteiner>)> &visit) {
    visit(node);
    for (const auto &c : node->children) preOrder(c, visit);
}

void GridSteiner::postOrder(const std::shared_ptr<GridSteiner> &node, const std::function<void(std::shared_ptr<GridSteiner>)> &visit) {
    for (const auto &c : node->children) postOrder(c, visit);
    visit(node);
}

void GridSteiner::postOrderCopy(const std::shared_ptr<GridSteiner> &node, const std::function<void(std::shared_ptr<GridSteiner>)> &visit) {
    auto tmp = node->children;
    for (const auto &c : tmp) postOrderCopy(c, visit);
    visit(node);
}

void GridSteiner::mergeNodes(const std::shared_ptr<GridSteiner> &root) {
    postOrderCopy(root, [](const std::shared_ptr<GridSteiner> &node) {
        // parent - node - child
        if (node->parent && node->parent->layerIdx == node->layerIdx && node->children.size() == 1 &&
            node->pinIdx < 0) {
            auto oldChild = node->children[0];
            if (node->layerIdx == oldChild->layerIdx && node->trackIdx == oldChild->trackIdx &&
                node->parent->trackIdx == node->trackIdx) {
                auto oldParent = node->parent;
                resetParent(node);
                resetParent(oldChild);
                setParent(oldChild, oldParent);
            }
        }
    });
}
