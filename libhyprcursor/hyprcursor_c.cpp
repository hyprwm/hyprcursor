#include "hyprcursor/hyprcursor.h"
#include "hyprcursor/hyprcursor.hpp"

using namespace Hyprcursor;

hyprcursor_manager_t* hyprcursor_manager_create(const char* theme_name) {
    return (hyprcursor_manager_t*)new CHyprcursorManager(theme_name);
}

hyprcursor_manager_t* hyprcursor_manager_create_with_logger(const char* theme_name, PHYPRCURSORLOGFUNC fn) {
    return (hyprcursor_manager_t*)new CHyprcursorManager(theme_name, fn);
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

struct SCursorImageData** hyprcursor_get_cursor_image_data(struct hyprcursor_manager_t* manager, const char* shape, struct hyprcursor_cursor_style_info info_, int* out_size) {
    const auto       MGR = (CHyprcursorManager*)manager;
    SCursorStyleInfo info;
    info.size                      = info_.size;
    int                       size = 0;
    struct SCursorImageData** data = MGR->getShapesC(size, shape, info);
    *out_size                      = size;
    return data;
}

void hyprcursor_cursor_image_data_free(hyprcursor_cursor_image_data** data, int size) {
    for (size_t i = 0; i < size; ++i) {
        free(data[i]);
    }

    free(data);
}

void hyprcursor_style_done(hyprcursor_manager_t* manager, hyprcursor_cursor_style_info info_) {
    const auto       MGR = (CHyprcursorManager*)manager;
    SCursorStyleInfo info;
    info.size = info_.size;
    return MGR->cursorSurfaceStyleDone(info);
}

void hyprcursor_register_logging_function(struct hyprcursor_manager_t* manager, PHYPRCURSORLOGFUNC fn) {
    const auto MGR = (CHyprcursorManager*)manager;
    MGR->registerLoggingFunction(fn);
}
