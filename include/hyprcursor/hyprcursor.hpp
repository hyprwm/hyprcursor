#pragma once

#include <vector>
#include <stdlib.h>
#include <string>

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
        C++ structs for hyprcursor_cursor_raw_shape_image and hyprcursor_cursor_raw_shape_data
    */
    struct SCursorRawShapeImage {
        std::vector<unsigned char> data;
        int                        size  = 0;
        int                        delay = 200;
    };

    struct SCursorRawShapeData {
        std::vector<SCursorRawShapeImage> images;
        float                             hotspotX    = 0;
        float                             hotspotY    = 0;
        std::string                       overridenBy = "";
        eHyprcursorResizeAlgo             resizeAlgo  = HC_RESIZE_NONE;
        eHyprcursorDataType               type        = HC_DATA_PNG;
    };

    /*!
        Basic Hyprcursor manager.

        Has to be created for either a specified theme, or
        nullptr if you want to use a default from the env.

        If no env is set, picks the first found.

        If none found, bool valid() will be false.

        If loading fails, bool valid() will be false.

        If theme has no valid cursor shapes, bool valid() will be false.
    */
    class CHyprcursorManager {
      public:
        CHyprcursorManager(const char* themeName);
        /*!
            \since 0.1.6
        */
        CHyprcursorManager(const char* themeName, PHYPRCURSORLOGFUNC fn);
        CHyprcursorManager(const char* themeName, PHYPRCURSORLOGFUNC fn, bool allowDefaultFallback);
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
            \since 0.1.6

            Returns the raw image data of a cursor shape, not rendered at all, alongside the metadata.
        */
        SCursorRawShapeData getRawShapeData(const char* shape_) {
            auto CDATA = getRawShapeDataC(shape_);

            if (CDATA->overridenBy) {
                SCursorRawShapeData d{.overridenBy = CDATA->overridenBy};
                free(CDATA->overridenBy);
                delete CDATA;
                return d;
            }

            SCursorRawShapeData data{.hotspotX = CDATA->hotspotX, .hotspotY = CDATA->hotspotY, .overridenBy = "", .resizeAlgo = CDATA->resizeAlgo, .type = CDATA->type};

            for (size_t i = 0; i < CDATA->len; ++i) {
                SCursorRawShapeImageC* cimage = &CDATA->images[i];
                SCursorRawShapeImage&  img    = data.images.emplace_back();
                img.size                      = cimage->size;
                img.delay                     = cimage->delay;
                img.data                      = std::vector<unsigned char>{(unsigned char*)cimage->data, (unsigned char*)cimage->data + (std::size_t)cimage->len};
            }

            delete[] CDATA->images;
            delete CDATA;

            return data;
        }

        /*!
            Prefer getShape, this is for C compat.
        */
        SCursorImageData** getShapesC(int& outSize, const char* shape_, const SCursorStyleInfo& info);

        /*!
            Prefer getShapeData, this is for C compat.
        */
        SCursorRawShapeDataC* getRawShapeDataC(const char* shape_);

        /*!
            Marks a certain style as done, allowing it to be potentially freed
        */
        void cursorSurfaceStyleDone(const SCursorStyleInfo&);

        /*!
            \since 0.1.6

            Registers a logging function to this manager.
            PHYPRCURSORLOGFUNC's msg is owned by the caller and will be freed afterwards.
            fn can be null to unregister a logger.
        */
        void registerLoggingFunction(PHYPRCURSORLOGFUNC fn);

      private:
        void                       init(const char* themeName_);

        CHyprcursorImplementation* impl                 = nullptr;
        bool                       finalizedAndValid    = false;
        bool                       allowDefaultFallback = true;
        PHYPRCURSORLOGFUNC         logFn                = nullptr;

        friend class CHyprcursorImplementation;
    };

}