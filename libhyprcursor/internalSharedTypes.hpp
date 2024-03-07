#pragma once
#include <string>
#include <vector>

enum eResizeAlgo {
    RESIZE_BILINEAR = 0,
    RESIZE_NEAREST  = 1,
};

struct SCursorImage {
    std::string filename;
    int         size = 0;
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