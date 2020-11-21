#include "CutLayer.h"
#include <tuple>

using namespace router::parser;

std::tuple<int64_t, int64_t, int64_t, int64_t> ViaType::getDefaultScore(int botDim, int topDim) const {
    return {bot[botDim].range(),     // belowWidth
            top[topDim].range(),     // aboveWidth
            bot[1 - botDim].range(), // belowWidth
            top[1 - topDim].range()};// aboveLength
}
