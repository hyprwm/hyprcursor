#pragma once
#include <string>
#include <vector>
#include <memory>
#include <hyprcursor/shared.h>

enum eShapeType {
    SHAPE_INVALID = 0,
    SHAPE_PNG,
    SHAPE_SVG,
};

inline eHyprcursorResizeAlgo stringToAlgo(const std::string& s) {
    if (s == "none")
        return HC_RESIZE_NONE;
    if (s == "nearest")
        return HC_RESIZE_NEAREST;
    return HC_RESIZE_BILINEAR;
}

inline std::string algoToString(const eHyprcursorResizeAlgo a) {
    switch (a) {
        case HC_RESIZE_BILINEAR: return "bilinear";
        case HC_RESIZE_NEAREST: return "nearest";
        case HC_RESIZE_NONE: return "none";
        default: return "none";
    }

    return "none";
}

struct SCursorImage {
    std::string filename;
    int         size  = 0;
    int         delay = 0;
};

struct SCursorShape {
    std::string               directory;
    float                     hotspotX = 0, hotspotY = 0, nominalSize = 1.F;
    eHyprcursorResizeAlgo     resizeAlgo = HC_RESIZE_NEAREST;
    std::vector<SCursorImage> images;
    std::vector<std::string>  overrides;
    eShapeType                shapeType = SHAPE_INVALID;
};

struct SCursorTheme {
    std::vector<std::unique_ptr<SCursorShape>> shapes;
};