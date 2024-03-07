#pragma once

#include "internalSharedTypes.hpp"
#include <optional>
#include <cairo/cairo.h>
#include <unordered_map>
#include <memory>

struct SLoadedCursorImage {
    ~SLoadedCursorImage() {
        if (data)
            delete[] (char*)data;
    }

    // read stuff
    size_t           readNeedle = 0;
    void*            data       = nullptr;
    size_t           dataLen    = 0;

    cairo_surface_t* cairoSurface = nullptr;
    int              side         = 0;
};

struct SLoadedCursorShape {
    std::vector<std::unique_ptr<SLoadedCursorImage>> images;
};

class CHyprcursorImplementation {
  public:
    std::string  themeName;
    std::string  themeFullDir;

    SCursorTheme theme;

    //
    std::unordered_map<SCursorShape*, SLoadedCursorShape> loadedShapes;

    //
    std::optional<std::string> loadTheme();
};