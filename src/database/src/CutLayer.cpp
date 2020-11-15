#include "CutLayer.h"
//#include "Setting.h"
//#include "Database.h"
#include <tuple>

using namespace database;

std::tuple<std::int64_t, std::int64_t, std::int64_t, std::int64_t> ViaType::getDefaultScore(const Dimension botDim, const Dimension topDim) const {
    return {bot[botDim].range(),     // belowWidth
            top[topDim].range(),     // aboveWidth
            bot[1 - botDim].range(), // belowWidth
            top[1 - topDim].range()};// aboveLength
}

utils::BoxT<std::int64_t> ViaType::getShiftedBotMetal(const utils::PointT<std::int64_t> &viaPos) const {
    utils::BoxT<std::int64_t> metal = bot;
    metal.ShiftBy(viaPos);
    return metal;
}
utils::BoxT<std::int64_t> ViaType::getShiftedTopMetal(const utils::PointT<std::int64_t> &viaPos) const {
    utils::BoxT<std::int64_t> metal = top;
    metal.ShiftBy(viaPos);
    return metal;
}
