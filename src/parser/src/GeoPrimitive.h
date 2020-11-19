#ifndef RL_ROUTER_GEOPRIMITIVE_H
#define RL_ROUTER_GEOPRIMITIVE_H
#include "functional"
#include "geo.h"


namespace router {
namespace parser {
enum Dimension {
    X = 0,
    Y = 1
};// end enum
//class Database;

//  BoxOnLayer
//  A box on a certain layer: primitive for route guide and pin acesss box

class BoxOnLayer : public BoxT<int64_t> {
public:
    int layerIdx{0};

    //  constructors
    template<typename... Args>
    explicit BoxOnLayer(int layerIndex = -1, Args... params) : layerIdx(layerIndex), BoxT<int64_t>(params...) {}

    // inherit setters from BoxT in batch
    template<typename... Args>
    void Set(int layerIndex = -1, Args... params) {
        layerIdx = layerIndex;
        BoxT<int64_t>::Set(params...);
    }

};

}// namespace parser
}// namespace router
#endif