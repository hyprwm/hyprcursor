#include <cairo/cairo.h>

#ifndef HYPRCURSOR_SHARED_H
#define HYPRCURSOR_SHARED_H

/*!
    struct for a single cursor image
*/
struct SCursorImageData {
    cairo_surface_t* surface;
    int              size;
    int              delay;
};

typedef struct SCursorImageData hyprcursor_cursor_image_data;

#endif
