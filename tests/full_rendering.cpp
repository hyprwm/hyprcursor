
/*
    full_rendering.cpp

    This example shows probably what you want to do.
    Hyprcursor will render a left_ptr shape at 48x48px to a file called /tmp/arrow.png
*/

#include <iostream>
#include <hyprcursor/hyprcursor.hpp>

void logFunction(enum eHyprcursorLogLevel level, char* message) {
    std::cout << "[hc] " << message << "\n";
}

int main(int argc, char** argv) {
    /*
        Create a manager. You can optionally pass a logger function.
    */
    Hyprcursor::CHyprcursorManager mgr(nullptr, logFunction);

    /*
        Manager could be invalid if no themes were found, or
        a specified theme was invalid.
    */
    if (!mgr.valid()) {
        std::cout << "mgr is invalid\n";
        return 1;
    }

    /*
        Style describes what pixel size you want your cursor
        images to be.

        Remember to free styles once you're done with them 
        (e.g. the user requested to change the cursor size to something else)
    */
    Hyprcursor::SCursorStyleInfo style{.size = 48};
    if (!mgr.loadThemeStyle(style)) {
        std::cout << "failed loading style\n";
        return 1;
    }

    /*
        Get a shape. This will return the data about available image(s),
        their delay, hotspot, etc.
    */
    const auto SHAPEDATA = mgr.getShape("left_ptr", style);

    /*
        If the size doesn't exist, images will be empty.
    */
    if (SHAPEDATA.images.empty()) {
        std::cout << "no images\n";
        return 1;
    }

    std::cout << "hyprcursor returned " << SHAPEDATA.images.size() << " images\n";

    /*
        Save to disk with cairo
    */
    const auto RET = cairo_surface_write_to_png(SHAPEDATA.images[0].surface, "/tmp/arrow.png");

    std::cout << "Cairo returned for write: " << RET << "\n";

    /*
        As mentioned before, clean up by releasing the style.
    */
    mgr.cursorSurfaceStyleDone(style);

    if (RET)
        return 1;

    return !mgr.valid();
}