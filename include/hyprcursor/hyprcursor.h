
#ifndef HYPRCURSOR_H
#define HYPRCURSOR_H

#ifdef __cplusplus

#define CAPI extern "C"

#else

#define CAPI

#endif

#include "shared.h"

struct hyprcursor_manager_t;

/*!
    Simple struct for styles
*/
struct hyprcursor_cursor_style_info {
    /*!
        Shape size.
        0 means "any" or "unspecified".
    */
    unsigned int size;
};

/*!
    Basic Hyprcursor manager.

    Has to be created for either a specified theme, or
    nullptr if you want to use a default from the env.

    If no env is set, picks the first found.

    If none found, hyprcursor_manager_valid will be false.

    If loading fails, hyprcursor_manager_valid will be false.

    The caller gets the ownership, call hyprcursor_manager_free to free this object.
*/
CAPI struct hyprcursor_manager_t* hyprcursor_manager_create(const char* theme_name);

/*!
    \since 0.1.6
    
    Same as hyprcursor_manager_create, but with a logger.
*/
CAPI struct hyprcursor_manager_t* hyprcursor_manager_create_with_logger(const char* theme_name, PHYPRCURSORLOGFUNC fn);

/*!
    Free a hyprcursor_manager_t*
*/
CAPI void hyprcursor_manager_free(struct hyprcursor_manager_t* manager);

/*!
    Returns true if the theme was successfully loaded,
    i.e. everything is A-OK and nothing should fail.
*/
CAPI int hyprcursor_manager_valid(struct hyprcursor_manager_t* manager);

/*!
    Loads a theme at a given style, synchronously.

    Returns whether it succeeded.
*/
CAPI int hyprcursor_load_theme_style(struct hyprcursor_manager_t* manager, struct hyprcursor_cursor_style_info info);

/*!
    Returns a hyprcursor_cursor_image_data*[] for a given cursor
    shape and size.

    The entire array needs to be freed instantly after using, see hyprcursor_cursor_image_data_free()

    Surfaces stay valid.

    Once done with a size, call hyprcursor_style_done()
*/
CAPI hyprcursor_cursor_image_data** hyprcursor_get_cursor_image_data(struct hyprcursor_manager_t* manager, const char* shape, struct hyprcursor_cursor_style_info info,
                                                                     int* out_size);

/*!
    Free a returned hyprcursor_cursor_image_data.
*/
CAPI void hyprcursor_cursor_image_data_free(hyprcursor_cursor_image_data** data, int size);

/*!
    Marks a certain style as done, allowing it to be potentially freed
*/
CAPI void hyprcursor_style_done(struct hyprcursor_manager_t* manager, struct hyprcursor_cursor_style_info info);

/*!
    \since 0.1.6

    Registers a logging function to a hyprcursor_manager_t*

    PHYPRCURSORLOGFUNC's msg is owned by the caller and will be freed afterwards.

    fn can be null to remove a logger.
*/
CAPI void hyprcursor_register_logging_function(struct hyprcursor_manager_t* manager, PHYPRCURSORLOGFUNC fn);

/*!
    \since 0.1.6

    Returns the raw image data of a cursor shape, not rendered at all, alongside the metadata.

    The object needs to be freed instantly after using, see hyprcursor_raw_shape_data_free()
*/
CAPI hyprcursor_cursor_raw_shape_data* hyprcursor_get_raw_shape_data(struct hyprcursor_manager_t* manager, char* shape);

/*!
    \since 0.1.6

    See hyprcursor_get_raw_shape_data.
    Frees the returned object.
*/
CAPI void hyprcursor_raw_shape_data_free(hyprcursor_cursor_raw_shape_data* data);

#endif