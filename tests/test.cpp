#include <iostream>
#include <hyprcursor.hpp>
#include <cairo/cairo.h>

int main(int argc, char** argv) {
    Hyprcursor::CHyprcursorManager mgr(nullptr);

    // get cursor for arrow
    const auto ARROW = mgr.getSurfaceFor("arrow", Hyprcursor::SCursorSurfaceInfo{.size = 64});

    // save to disk
    const auto RET = cairo_surface_write_to_png(ARROW, "/tmp/arrow.png");

    std::cout << "Cairo returned for write: " << RET << "\n";

    return !mgr.valid();
}