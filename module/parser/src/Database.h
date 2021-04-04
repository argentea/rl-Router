#pragma once
#include "CutLayer.h"
#include "MetalLayer.h"
#include "Net.h"

namespace router::parser {

class Database {
public:
    std::vector<MetalLayer> _metal_layers;
    std::vector<CutLayer> _cut_layers;
    std::vector<ParserNet> _nets;

	void init();
};
}// namespace router::parser
