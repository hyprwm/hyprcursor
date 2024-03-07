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
        if (artificialData)
            delete[] (char*)artificialData;
        if (cairoSurface)
            cairo_surface_destroy(cairoSurface);
    }

    // read stuff
    size_t           readNeedle = 0;
    void*            data       = nullptr;
    size_t           dataLen    = 0;

    cairo_surface_t* cairoSurface = nullptr;
    int              side         = 0;
    int delay = 0;

    // means this was created by resampling
    void* artificialData = nullptr;
    bool  artificial     = false;
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