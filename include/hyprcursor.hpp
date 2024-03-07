#pragma once

#include <cairo/cairo.h>

class CHyprcursorImplementation;

namespace Hyprcursor {

    /*!
        Simple struct for some info about shape requests
    */
    struct SCursorSurfaceInfo {
        /*
            Shape size
        */
        unsigned int size = 0;
    };

    /*!
        Basic Hyprcursor manager.

        Has to be created for either a specified theme, or
        nullptr if you want to use a default from the env.

        If no env is set, picks the first found.

        If none found, bool valid() will be false.

        If loading fails, bool valid() will be false.
    */
    class CHyprcursorManager {
      public:
        CHyprcursorManager(const char* themeName);
        ~CHyprcursorManager();

        /*!
            Returns true if the theme was successfully loaded,
            i.e. everything is A-OK and nothing should fail.
        */
        bool valid();

        /*!
            Returns a cairo_surface_t for a given cursor
            shape and size.

            Once done, call cursorSurfaceDone()
        */
        cairo_surface_t* getSurfaceFor(const char* shape, const SCursorSurfaceInfo& info);

        /*!
            Marks a surface as done, meaning ready to be freed.

            Always call after using a surface.
        */
        void cursorSurfaceDone(cairo_surface_t* surface);

      private:
        CHyprcursorImplementation* impl              = nullptr;
        bool                       finalizedAndValid = false;
    };

}