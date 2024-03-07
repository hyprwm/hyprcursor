
#ifndef HYPRCURSOR_H
#define HYPRCURSOR_H

#ifdef __cplusplus

#define CAPI extern "C"

#else

#define CAPI

#endif

struct hyprcursor_manager_t;

/*!
    Basic Hyprcursor manager.

    Has to be created for either a specified theme, or
    nullptr if you want to use a default from the env.

    If no env is set, picks the first found.

    If none found, hyprcursor_manager_valid will be false.

    If loading fails, hyprcursor_manager_valid will be false.

    The caller gets the ownership, call hyprcursor_manager_free to free this object.
*/
CAPI hyprcursor_manager_t* hyprcursor_manager_create(const char* theme_name);

/*!
    Free a hyprcursor_manager_t*
*/
CAPI void hyprcursor_manager_free(hyprcursor_manager_t* manager);

/*!
    Returns true if the theme was successfully loaded,
    i.e. everything is A-OK and nothing should fail.
*/
CAPI bool hyprcursor_manager_valid(hyprcursor_manager_t* manager);

#endif