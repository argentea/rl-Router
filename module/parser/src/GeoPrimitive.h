#pragma once
#include "functional"
#include "geo.h"


namespace router::parser {

//  BoxOnLayer
//  A box on a certain layer: primitive for route guide and pin acesss box
class BoxOnLayer : public BoxT<int64_t> {
public:
    int _layer_idx{-1};

    //  constructors
    template<typename... Args>
    explicit BoxOnLayer(int layerIndex = -1, Args... params) : BoxT<int64_t>(params...), _layer_idx(layerIndex){};

    // inherit setters from BoxT in batch
    template<typename... Args>
    void Set(Args... params) {
        BoxT<int64_t>::Set(params...);
    }
};

}// namespace router::parser