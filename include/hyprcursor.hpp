#pragma once

#include <cairo/cairo.h>

class CHyprcursorImplementation;

namespace Hyprcursor {

    /*!
        Simple struct for styles
    */
    struct SCursorStyleInfo {
        /*!
            Shape size.

            0 means "any" or "unspecified".
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
            Loads this theme at a given style, synchronously.

            Returns whether it succeeded.
        */
        bool loadThemeStyle(const SCursorStyleInfo& info);

        /*!
            Returns a cairo_surface_t for a given cursor
            shape and size.

            Once done with a size, call cursorSurfaceDone()
        */
        cairo_surface_t* getSurfaceFor(const char* shape, const SCursorStyleInfo& info);

        /*!
            Marks a certain style as done, allowing it to be potentially freed
        */
        void cursorSurfaceStyleDone(const SCursorStyleInfo&);

      private:
        CHyprcursorImplementation* impl              = nullptr;
        bool                       finalizedAndValid = false;
    };

}