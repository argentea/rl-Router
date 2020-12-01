#pragma once
#include "Layer.h"

namespace router::db {

class LayerList {
public:
    Dimension getLayerDir(int layerIdx) const { return layers[layerIdx].direction; }

    const MetalLayer &getLayer(int layerIdx) const { return layers[layerIdx]; }

    //    const CutLayer& getCutLayer(int cutLayerIdx) const { return cutLayers[cutLayerIdx]; }
    unsigned getLayerNum() const noexcept { return layers.size(); }

protected:
    std::vector<MetalLayer> layers;

    int numGridPoints;
    int64_t totalTrackLength;
    int numVias;
};

}// namespace router::db
