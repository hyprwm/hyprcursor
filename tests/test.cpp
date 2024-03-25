#include <iostream>
#include <hyprcursor/hyprcursor.hpp>

void logFunction(enum eHyprcursorLogLevel level, char* message) {
    std::cout << "[hc] " << message << "\n";
}

/*
    hyprlang-test in C++.
    Renders a cursor shape to /tmp at 48px
*/

int main(int argc, char** argv) {
    Hyprcursor::CHyprcursorManager mgr("classicFlatHypr", logFunction);

    if (!mgr.valid()) {
        std::cout << "mgr is invalid\n";
        return 1;
    }

    Hyprcursor::SCursorStyleInfo style{.size = 48};
    // preload size 48 for testing
    if (!mgr.loadThemeStyle(style)) {
        std::cout << "failed loading style\n";
        return 1;
    }

    // get cursor for left_ptr
    const auto SHAPEDATA = mgr.getShape("left_ptr", style);

    if (SHAPEDATA.images.empty()) {
        std::cout << "no images\n";
        return 1;
    }

    std::cout << "hyprcursor returned " << SHAPEDATA.images.size() << " images\n";

    // save to disk
    const auto RET = cairo_surface_write_to_png(SHAPEDATA.images[0].surface, "/tmp/arrow.png");

    std::cout << "Cairo returned for write: " << RET << "\n";

    mgr.cursorSurfaceStyleDone(style);

    if (RET)
        return 1;

    return !mgr.valid();
}