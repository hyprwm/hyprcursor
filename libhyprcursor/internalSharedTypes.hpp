#pragma once
#include <string>
#include <vector>
#include <memory>

enum eResizeAlgo {
    RESIZE_NONE     = 0,
    RESIZE_BILINEAR = 1,
    RESIZE_NEAREST  = 2,
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