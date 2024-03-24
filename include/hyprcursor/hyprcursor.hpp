#pragma once

#include <vector>
#include <stdlib.h>

#include "shared.h"

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
        struct for cursor shape data
    */
    struct SCursorShapeData {
        std::vector<SCursorImageData> images;
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
        /*!
            \since 0.1.5
        */
        CHyprcursorManager(const char* themeName, PHYPRCURSORLOGFUNC fn);
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
            Returns the shape data struct for a given
            style.

            Once done with a style, call cursorSurfaceDone()

            The surfaces references stay valid until cursorSurfaceStyleDone() is called on the owning style.
        */
        SCursorShapeData getShape(const char* shape, const SCursorStyleInfo& info) {
            int                size   = 0;
            SCursorImageData** images = getShapesC(size, shape, info);

            SCursorShapeData   data;

            for (size_t i = 0; i < size; ++i) {
                SCursorImageData image;
                image.delay    = images[i]->delay;
                image.size     = images[i]->size;
                image.surface  = images[i]->surface;
                image.hotspotX = images[i]->hotspotX;
                image.hotspotY = images[i]->hotspotY;
                data.images.push_back(image);

                free(images[i]);
            }

            free(images);

            return data;
        }

        /*!
            Prefer getShape, this is for C compat.
        */
        SCursorImageData** getShapesC(int& outSize, const char* shape_, const SCursorStyleInfo& info);

        /*!
            Marks a certain style as done, allowing it to be potentially freed
        */
        void cursorSurfaceStyleDone(const SCursorStyleInfo&);

        /*!
            \since 0.1.5

            Registers a logging function to this manager.
            PHYPRCURSORLOGFUNC's msg is owned by the caller and will be freed afterwards.
            fn can be null to unregister a logger.
        */
        void registerLoggingFunction(PHYPRCURSORLOGFUNC fn);

      private:
        void                       init(const char* themeName_);

        CHyprcursorImplementation* impl              = nullptr;
        bool                       finalizedAndValid = false;
        PHYPRCURSORLOGFUNC         logFn             = nullptr;

        friend class CHyprcursorImplementation;
    };

}