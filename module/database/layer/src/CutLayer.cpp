#include "CutLayer.h"
#include "parser/src/CutLayer.h"
#include "logger/src/Logger.h"
using std::cout;

namespace db {

ViaType::ViaType(router::parser::ViaType& parserViaType) {

    bot = parserViaType.bot;
    cut = parserViaType.cut;
    top = parserViaType.top;
    name = parserViaType.name;

    if (!bot.IsStrictValid() || !cut.IsStrictValid() || !top.IsStrictValid()) {
		std::cout << "Warning in " << __func__ << ": For " << parserViaType.name
              << " , has non strict valid via layer bound... " << std::endl;
    }
}

std::tuple<DBU, DBU, DBU, DBU> ViaType::getDefaultScore(const Dimension botDim, const Dimension topDim) const {
    return std::tuple<DBU, DBU, DBU, DBU>(bot[botDim].range(),       // belowWidth
                                          top[topDim].range(),       // aboveWidth
                                          bot[1 - botDim].range(),   // belowWidth
                                          top[1 - topDim].range());  // aboveLength
}

utils::BoxT<DBU> ViaType::getShiftedBotMetal(const utils::PointT<DBU>& viaPos) const {
    utils::BoxT<DBU> metal = bot;
    metal.ShiftBy(viaPos);
    return metal;
}
utils::BoxT<DBU> ViaType::getShiftedTopMetal(const utils::PointT<DBU>& viaPos) const {
    utils::BoxT<DBU> metal = top;
    metal.ShiftBy(viaPos);
    return metal;
}

CutLayer::CutLayer(const router::parser::CutLayer& parserLayer)
    : name(parserLayer.name), idx(parserLayer.idx), width(parserLayer.width) {
	for(auto pViatype : parserLayer.allViaTypes){
		allViaTypes.emplace_back(pViatype);
	}
}

ostream& CutLayer::printBasics(ostream& os) const {
    os << name << ": idx=" << idx << ", viaTypes=" << defaultViaType().name << " (";
    for (auto via : allViaTypes) {
        if (via.name != defaultViaType().name) {
            os << via.name << " ";
        }
    }
    os << ")";
    return os;
}

ostream& CutLayer::printDesignRules(ostream& os) const {
    os << name << ": width=" << width << ", space=" << spacing;
    return os;
}

ostream& CutLayer::printViaOccupancyLUT(ostream& os) const {
    os << name << ": viaCut(" << viaCut().size() / 2 + 1 << ',' << viaCut()[0].size() / 2 + 1 << ")";
    os << ", viaMetal(" << viaMetal().size() / 2 + 1 << ',' << viaMetal()[0].size() / 2 + 1 << ")";
    // TODO: make xSize member variables, since they will be the same in a LUT over all cps
    auto getMaxSize = [](const vector<vector<vector<bool>>>& LUT, size_t& xSize, size_t& ySize) {
        xSize = 0;
        ySize = 0;
        for (const vector<vector<bool>>& cpLUT : LUT) {
            if (cpLUT.size()) {
                xSize = max(xSize, cpLUT.size());
                ySize = max(ySize, cpLUT[0].size());
            }
        }
    };
    size_t xSize, ySize;
    os << ", viaBotVia(";
    if (defaultViaType().allViaBotVia.size()) {
        getMaxSize(viaBotVia(), xSize, ySize);
        os << viaBotVia().size() << ',' << xSize << ',' << ySize << ")";
    }
    else os << "-,-,-)";
    os << ", viaTopVia(";
    if (defaultViaType().allViaTopVia.size()) {
        getMaxSize(viaTopVia(), xSize, ySize);
        os << viaTopVia().size() << ',' << xSize << ',' << ySize << ")";
    }
    else os << "-,-,-)";
    getMaxSize(viaBotWire(), xSize, ySize);
    os << ", viaBotWire(" << viaBotWire().size() << ',' << xSize << ',' << ySize << ")";
    getMaxSize(viaTopWire(), xSize, ySize);
    os << ", viaTopWire(" << viaTopWire().size() << ',' << xSize << ',' << ySize << ")";
    return os;
}

ostream& operator<<(ostream& os, const CutLayer& layer) { return layer.printBasics(os); }

}  //   namespace db
