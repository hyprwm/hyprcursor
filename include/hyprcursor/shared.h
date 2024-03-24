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
    int              hotspotX;
    int              hotspotY;
};

typedef struct SCursorImageData hyprcursor_cursor_image_data;

enum eHyprcursorLogLevel {
    HC_LOG_NONE = 0,
    HC_LOG_TRACE,
    HC_LOG_INFO,
    HC_LOG_WARN,
    HC_LOG_ERR,
    HC_LOG_CRITICAL,
};

/*
    msg is owned by the caller and will be freed afterwards.
*/
typedef void (*PHYPRCURSORLOGFUNC)(enum eHyprcursorLogLevel level, char* msg);

#endif
