#ifndef RL_ROUTER_DATABASE_H
#define RL_ROUTER_DATABASE_H
#include "CutLayer.h"
#include "MetalLayer.h"
#include "Net.h"

namespace router {
namespace parser {

class Database {
public:
    std::vector<MetalLayer> _metal_layers;
    std::vector<CutLayer> _cut_layers;
    std::vector<Net> _nets;
};
}// namespace parser
}// namespace router

#endif//RL_ROUTER_DATABASE_H
