#include <hyprcursor/hyprcursor.h>
#include <stdio.h>
#include <stdlib.h>

void logFunction(enum eHyprcursorLogLevel level, char* message) {
    printf("[hc] %s\n", message);
}

/*
    hyprlang-test in C.
    Renders a cursor shape to /tmp at 48px
*/

int main(int argc, char** argv) {
    struct hyprcursor_manager_t* mgr = hyprcursor_manager_create_with_logger(NULL, logFunction);

    if (!mgr) {
        printf("mgr null\n");
        return 1;
    }

    if (!hyprcursor_manager_valid(mgr)) {
        printf("mgr is invalid\n");
        return 1;
    }

    struct hyprcursor_cursor_style_info info = {.size = 48};
    if (!hyprcursor_load_theme_style(mgr, info)) {
        printf("load failed\n");
        return 1;
    }

    int                            dataSize = 0;
    hyprcursor_cursor_image_data** data     = hyprcursor_get_cursor_image_data(mgr, "left_ptr", info, &dataSize);
    if (data == NULL) {
        printf("data failed\n");
        return 1;
    }

    int ret = cairo_surface_write_to_png(data[0]->surface, "/tmp/arrowC.png");

    hyprcursor_cursor_image_data_free(data, dataSize);
    hyprcursor_style_done(mgr, info);

    if (ret) {
        printf("cairo failed\n");
        return 1;
    }

    return 0;
}