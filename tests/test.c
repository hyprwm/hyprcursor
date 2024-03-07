#include <hyprcursor.h>
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char** argv) {
    struct hyprcursor_manager_t* mgr = hyprcursor_manager_create(NULL);

    if (!mgr) {
        printf("mgr null\n");
        return 1;
    }

    struct hyprcursor_cursor_style_info info = {.size = 48};
    if (!hyprcursor_load_theme_style(mgr, info)) {
        printf("load failed\n");
        return 1;
    }

    cairo_surface_t* surf = hyprcursor_get_surface_for(mgr, "left_ptr", info);
    if (surf == NULL) {
        printf("surf failed\n");
        return 1;
    }

    int ret = cairo_surface_write_to_png(surf, "/tmp/arrowC.png");

    if (ret) {
        printf("cairo failed\n");
        return 1;
    }

    return 0;
}