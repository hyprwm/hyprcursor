
/*
    only_metadata.cpp

    This is a mode in which you probably do NOT want to operate,
    but major DEs might want their own renderer for
    cursor shapes.

    Prefer full_rendering.cpp for consistency and simplicity.
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
        If you are planning on using your own renderer,
        you do not want to load in any styles, as those
        are rendered once you make your call.

        Instead, let's request left_ptr's metadata
    */
    auto RAWDATA = mgr.getRawShapeData("left_ptr");

    /*
        if images are empty, check overridenBy
    */
    if (RAWDATA.images.empty()) {

        /*
            if overridenBy is empty, the current theme doesn't have this shape.
        */
        if (RAWDATA.overridenBy.empty())
            return false;

        /*
            load what it's overriden by.
        */
        RAWDATA = mgr.getRawShapeData(RAWDATA.overridenBy.c_str());
    }

    /*
        If we still have no images, the theme seems broken.
    */
    if (RAWDATA.images.empty()) {
        std::cout << "failed querying left_ptr\n";
        return 1;
    }

    /*
        You can query the images (animation frames)
        or their properties.

        Every image has .data and .type for you to handle.
    */
    std::cout << "left_ptr images: " << RAWDATA.images.size() << "\n";
    for (auto& i : RAWDATA.images)
        std::cout << "left_ptr data size: " << i.data.size() << "\n";

    
    return 0;
}