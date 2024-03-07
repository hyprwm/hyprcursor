#include <iostream>
#include <hyprcursor/hyprcursor.hpp>

int main(int argc, char** argv) {
    Hyprcursor::CHyprcursorManager mgr(nullptr);

    // preload size 48 for testing
    if (!mgr.loadThemeStyle(Hyprcursor::SCursorStyleInfo{.size = 48})) {
        std::cout << "failed loading style\n";
        return 1;
    }

    // get cursor for left_ptr
    const auto SHAPEDATA = mgr.getShape("wait", Hyprcursor::SCursorStyleInfo{.size = 48});

    if (SHAPEDATA.images.empty()) {
        std::cout << "no images\n";
        return 1;
    }

    std::cout << "hyprcursor returned " << SHAPEDATA.images.size() << " images\n";

    // save to disk
    const auto RET = cairo_surface_write_to_png(SHAPEDATA.images[0].surface, "/tmp/arrow.png");

    std::cout << "Cairo returned for write: " << RET << "\n";

    if (RET)
        return 1;

    return !mgr.valid();
}