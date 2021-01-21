#pragma once
#include "Layer.h"
#include "parser/src/Parser.h"
#include <vector>

namespace db {

class LayerList {
public:
	void init(router::parser::Parser& parser);
    Dimension getLayerDir(int layerIdx) const { return _layers[layerIdx].direction; }

    const MetalLayer &getLayer(int layerIdx) const { return _layers[layerIdx]; }

    //    const CutLayer& getCutLayer(int cutLayerIdx) const { return cutLayers[cutLayerIdx]; }
    unsigned getLayerNum() const noexcept { return _layers.size(); }

protected:
    std::vector<MetalLayer> _layers;
	std::vector<MetalLayer> _cut_layers;

    int numGridPoints;
    int64_t totalTrackLength;
    int numVias;
};

}// namespace router::db
