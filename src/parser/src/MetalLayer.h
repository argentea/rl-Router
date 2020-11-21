#ifndef RL_ROUTER_METALLAYER_H
#define RL_ROUTER_METALLAYER_H

#include "GeoPrimitive.h"

namespace router {
namespace parser {

enum Direction {
    X = 0,
    Y = 1
};

class Track {
public:
    explicit Track(int64_t loc) : _location(loc) {}
    int64_t _location;
};

class SpaceRule {
public:
    SpaceRule(const int64_t space, const int64_t eolWidth, const int64_t eolWithin)
        : _space(space), _has_eol(true), _eol_width(eolWidth), _eol_within(eolWithin) {}
    SpaceRule(const int64_t space, const int64_t eolWidth, const int64_t eolWithin, const int64_t parSpace, const int64_t parWithin)
        : _space(space),
          _has_eol(true),
          _eol_width(eolWidth),
          _eol_within(eolWithin),
          _has_par(true),
          _par_space(parSpace),
          _par_within(parWithin) {}

    int64_t _space = 0;
    bool _has_eol = false;
    int64_t _eol_width = 0;
    int64_t _eol_within = 0;
    bool _has_par = false;
    int64_t _par_space = 0;
    int64_t _par_within = 0;
};


// note: for operations on GeoPrimitives, all checking is down in LayerList for low level efficiency
class MetalLayer {
public:
    // Basic infomation
    std::string _name;
    Direction _direction;// direction of track dimension
    int _idx;            // layerIdx (consistent with Rsyn::xxx::getRelativeIndex())

    // Track (1D)
    int64_t _pitch = 0;
    std::vector<Track> _tracks;

    // Design rules
    // _width
    int64_t _width = 0;
    int64_t _min_width = 0;
    int64_t _width_for_suff_ovlp = 0;
    int64_t _shrink_for_suff_ovlp = 0;
    // minArea
    int64_t _min_area = 0;
    int64_t _min_len_raw = 0;
    // parallel spacing
    std::vector<int64_t> _parallel_width{0};
    std::vector<int64_t> _parallel_length{0};
    std::vector<std::vector<int64_t>> _parallel_width_space{{0}};
    int64_t _default_space = 0;
    int64_t _para_run_space_for_larger_width = 0;
    int64_t getParaRunSpace(int64_t width, int64_t length = 0) const;
    // eol spacing
    // TODO: handle multiple spaceRules
    std::vector<SpaceRule> _space_rules;
    int64_t _max_eol_space = 0;
    int64_t _max_eol_width = 0;
    int64_t _max_eol_within = 0;
    // corner spacing
    bool _corner_except_eol = false;
    int64_t _corner_eol_width = 0;
    std::vector<int64_t> _corner_width{0};
    std::vector<int64_t> _corner_width_space{0};
    bool hasCornerSpace() const { return _corner_width_space.size() > 1 || _corner_width_space[0]; }

    // Translate design rule
    // either both parallel-run spacing or eol spacing
    // there is parallel-run spacing with negative length if the targetMetal is not eol dominated
    // margin for multi-thread safe and others
    int64_t fixedMetalQueryMargin = 0;
};

}// namespace parser
}// namespace router

#endif