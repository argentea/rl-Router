#ifndef RL_ROUTER_NET_H
#define RL_ROUTER_NET_H

namespace router {
namespace parser {


class Net {
public:
    int idx;
    std::string _net_name;
    std::vector<std::vector<BoxOnLayer>> pinAccessBoxes;// (pinIdx, accessBoxIdx) -> BoxOnLayer

    // route guides
    std::vector<BoxOnLayer> routeGuides;

    // more route guide information
    std::vector<int> routeGuideVios;
};

class NetList {
public:
    std::vector<Net> nets;
};

}// namespace parser
}// namespace router

#endif