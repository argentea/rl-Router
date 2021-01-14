#pragma once
#include <string>
#include <vector>

namespace router::parser {

// net index
// a valid net idx >= 0
enum netIndex {
    OBS_NET_IDX = -1, // for obstacles
    NULL_NET_IDX = -2,// for neither net nor obstacle
};

class Net {
public:
    int _idx;
    std::string _net_name;
    std::vector<std::vector<BoxOnLayer>> _pin_access_boxes;// (pinIdx, accessBoxIdx) -> BoxOnLayer

    // route guides
    std::vector<BoxOnLayer> _route_guides;

    // more route guide information
    std::vector<int> _route_guide_vios;
};
}// namespace router::parser
