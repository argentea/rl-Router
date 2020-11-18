#include "CutLayer.h"
//#include "Setting.h"
//#include "Database.h"
#include <tuple>

using namespace router::parser;

std::tuple<int64_t, int64_t, int64_t, int64_t> ViaType::getDefaultScore(const Dimension botDim, const Dimension topDim) const {
    return {bot[botDim].range(),     // belowWidth
            top[topDim].range(),     // aboveWidth
            bot[1 - botDim].range(), // belowWidth
            top[1 - topDim].range()};// aboveLength
}

BoxT<int64_t> ViaType::getShiftedBotMetal(const PointT<int64_t> &viaPos) const {
    BoxT<int64_t> metal = bot;
    metal.ShiftBy(viaPos);
    return metal;
}
BoxT<int64_t> ViaType::getShiftedTopMetal(const PointT<int64_t> &viaPos) const {
    BoxT<int64_t> metal = top;
    metal.ShiftBy(viaPos);
    return metal;
}
