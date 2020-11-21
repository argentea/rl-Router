#include "MetalLayer.h"

using namespace router::parser;

int64_t MetalLayer::getParaRunSpace(const int64_t width, const int64_t length) const {
    int iWidth = parallelWidth.size() - 1;// first smaller than or equal to
    while (iWidth > 0 && parallelWidth[iWidth] >= width) {
        --iWidth;
    }
    if (length == 0) return parallelWidthSpace[iWidth][0];// fast return
    int iLength = parallelLength.size() - 1;              // first smaller than or equal to
    while (iLength > 0 && parallelLength[iLength] >= length) {
        --iLength;
    }
    return parallelWidthSpace[iWidth][iLength];
}
