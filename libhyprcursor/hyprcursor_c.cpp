#include "hyprcursor.h"
#include "hyprcursor.hpp"

using namespace Hyprcursor;

hyprcursor_manager_t* hyprcursor_manager_create(const char* theme_name) {
    return (hyprcursor_manager_t*)new CHyprcursorManager(theme_name);
}

void hyprcursor_manager_free(hyprcursor_manager_t* manager) {
    delete (CHyprcursorManager*)manager;
}

bool hyprcursor_manager_valid(hyprcursor_manager_t* manager) {
    const auto MGR = (CHyprcursorManager*)manager;
    return MGR->valid();
}