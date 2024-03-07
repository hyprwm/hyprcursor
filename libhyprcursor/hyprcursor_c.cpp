#include "hyprcursor.h"
#include "hyprcursor.hpp"

using namespace Hyprcursor;

hyprcursor_manager_t* hyprcursor_manager_create(const char* theme_name) {
    return (hyprcursor_manager_t*)new CHyprcursorManager(theme_name);
}

void hyprcursor_manager_free(hyprcursor_manager_t* manager) {
    delete (CHyprcursorManager*)manager;
}

int hyprcursor_manager_valid(hyprcursor_manager_t* manager) {
    const auto MGR = (CHyprcursorManager*)manager;
    return MGR->valid();
}

int hyprcursor_load_theme_style(hyprcursor_manager_t* manager, hyprcursor_cursor_style_info info_) {
    const auto       MGR = (CHyprcursorManager*)manager;
    SCursorStyleInfo info;
    info.size = info_.size;
    return MGR->loadThemeStyle(info);
}

cairo_surface_t* hyprcursor_get_surface_for(hyprcursor_manager_t* manager, const char* shape, hyprcursor_cursor_style_info info_) {
    const auto       MGR = (CHyprcursorManager*)manager;
    SCursorStyleInfo info;
    info.size = info_.size;
    return MGR->getSurfaceFor(shape, info);
}

void hyprcursor_style_done(hyprcursor_manager_t* manager, hyprcursor_cursor_style_info info_) {
    const auto       MGR = (CHyprcursorManager*)manager;
    SCursorStyleInfo info;
    info.size = info_.size;
    return MGR->cursorSurfaceStyleDone(info);
}