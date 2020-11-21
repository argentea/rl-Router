#include "MetalLayer.h"

using namespace router::parser;

int64_t MetalLayer::getParaRunSpace(const int64_t width, const int64_t length) const {
    int iWidth = static_cast<int>(_parallel_width.size()) - 1;// first smaller than or equal to
    while (iWidth > 0 && _parallel_width[iWidth] >= width) {
        --iWidth;
    }
    if (length == 0) return _parallel_width_space[iWidth][0];// fast return
    int iLength = static_cast<int>(_parallel_length.size()) - 1;              // first smaller than or equal to
    while (iLength > 0 && _parallel_length[iLength] >= length) {
        --iLength;
    }
    return _parallel_width_space[iWidth][iLength];
}
