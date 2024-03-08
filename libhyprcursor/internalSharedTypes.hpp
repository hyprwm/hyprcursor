#pragma once
#include <string>
#include <vector>
#include <memory>

enum eResizeAlgo {
    RESIZE_INVALID = 0,
    RESIZE_NONE,
    RESIZE_BILINEAR,
    RESIZE_NEAREST,
};

enum eShapeType {
    SHAPE_INVALID = 0,
    SHAPE_PNG,
    SHAPE_SVG,
};

inline eResizeAlgo stringToAlgo(const std::string& s) {
    if (s == "none")
        return RESIZE_NONE;
    if (s == "nearest")
        return RESIZE_NEAREST;
    return RESIZE_BILINEAR;
}

inline std::string algoToString(const eResizeAlgo a) {
    switch (a) {
        case RESIZE_BILINEAR: return "bilinear";
        case RESIZE_NEAREST: return "nearest";
        case RESIZE_NONE: return "none";
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
    float                     hotspotX = 0, hotspotY = 0;
    eResizeAlgo               resizeAlgo = RESIZE_NEAREST;
    std::vector<SCursorImage> images;
    std::vector<std::string>  overrides;
    eShapeType                shapeType = SHAPE_INVALID;
};

struct SCursorTheme {
    std::vector<std::unique_ptr<SCursorShape>> shapes;
};