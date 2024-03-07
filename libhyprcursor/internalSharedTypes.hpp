#pragma once
#include <string>
#include <vector>

enum eResizeAlgo {
    RESIZE_NONE     = 0,
    RESIZE_BILINEAR = 1,
    RESIZE_NEAREST  = 2,
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
    int         size = 0;
    int         delay = 0;
};

struct SCursorShape {
    std::string               directory;
    float                     hotspotX = 0, hotspotY = 0;
    eResizeAlgo               resizeAlgo = RESIZE_NEAREST;
    std::vector<SCursorImage> images;
    std::vector<std::string>  overrides;
};

struct SCursorTheme {
    std::vector<SCursorShape> shapes;
};