#include <iostream>
#include <hyprcursor.hpp>

int main(int argc, char** argv) {
    Hyprcursor::CHyprcursorManager mgr(nullptr);

    // preload size 48 for testing
    if (!mgr.loadThemeStyle(Hyprcursor::SCursorStyleInfo{.size = 48})) {
        std::cout << "failed loading style\n";
        return 1;
    }

    // get cursor for left_ptr
    const auto SHAPEDATA = mgr.getShape("left_ptr", Hyprcursor::SCursorStyleInfo{.size = 48});

    if (SHAPEDATA.images.empty())   
        return 1;

    // save to disk
    const auto RET = cairo_surface_write_to_png(SHAPEDATA.images[0].surface, "/tmp/arrow.png");

    std::cout << "Cairo returned for write: " << RET << "\n";

    if (RET)
        return 1;

    return !mgr.valid();
}