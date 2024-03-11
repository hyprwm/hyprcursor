cmake_minimum_required(VERSION 3.19)

set(HYPRCURSOR_VERSION "0.1.2")
add_compile_definitions(HYPRCURSOR_VERSION="${HYPRCURSOR_VERSION}")

project(hyprcursor
    VERSION ${HYPRCURSOR_VERSION}
    DESCRIPTION "A library and toolkit for the Hyprland cursor format"
)

include(CTest)
include(GNUInstallDirs)

set(PREFIX ${CMAKE_INSTALL_PREFIX})
set(INCLUDE ${CMAKE_INSTALL_FULL_INCLUDEDIR})
set(LIBDIR ${CMAKE_INSTALL_FULL_LIBDIR})

configure_file(hyprcursor.pc.in hyprcursor.pc @ONLY)

set(CMAKE_CXX_STANDARD 23)

find_package(PkgConfig REQUIRED)
pkg_check_modules(deps REQUIRED IMPORTED_TARGET hyprlang>=0.4.0 libzip cairo librsvg-2.0)

if(CMAKE_BUILD_TYPE MATCHES Debug OR CMAKE_BUILD_TYPE MATCHES DEBUG)
    message(STATUS "Configuring hyprcursor in Debug")
    add_compile_definitions(HYPRLAND_DEBUG)
else()
    add_compile_options(-O3)
    message(STATUS "Configuring hyprcursor in Release")
endif()

file(GLOB_RECURSE SRCFILES CONFIGURE_DEPENDS "libhyprcursor/*.cpp" "include/hyprcursor/hyprcursor.hpp" "include/hyprcursor/hyprcursor.h" "include/hyprcursor/shared.h")

add_library(hyprcursor SHARED ${SRCFILES})
target_include_directories( hyprcursor
    PUBLIC "./include"
    PRIVATE "./libhyprcursor"
)
set_target_properties(hyprcursor PROPERTIES
    VERSION ${hyprcursor_VERSION}
    SOVERSION 0
    PUBLIC_HEADER include/hyprcursor/hyprcursor.hpp include/hyprcursor/hyprcursor.h include/hyprcursor/shared.h
)

target_link_libraries(hyprcursor PkgConfig::deps)

if (CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    # for std::expected.
    # probably evil. Arch's clang is very outdated tho...
    target_compile_options(hyprcursor PUBLIC -std=gnu++2b -D__cpp_concepts=202002L -Wno-macro-redefined)
endif()

# hyprcursor-util
add_subdirectory(hyprcursor-util)

install(TARGETS hyprcursor)

# tests
add_custom_target(tests)

add_executable(hyprcursor_test "tests/test.cpp")
target_link_libraries(hyprcursor_test PRIVATE hyprcursor)
add_test(NAME "Test libhyprcursor in C++" WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}/tests COMMAND hyprcursor_test)
add_dependencies(tests hyprcursor_test)

add_executable(hyprcursor_test_c "tests/test.c")
target_link_libraries(hyprcursor_test_c PRIVATE hyprcursor)
add_test(NAME "Test libhyprcursor in C" WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}/tests COMMAND hyprcursor_test_c)
add_dependencies(tests hyprcursor_test_c)

# Installation
install(DIRECTORY "include/hyprcursor" DESTINATION ${CMAKE_INSTALL_INCLUDEDIR} DIRECTORY_PERMISSIONS 
    OWNER_WRITE OWNER_READ OWNER_EXECUTE
    GROUP_WRITE GROUP_READ GROUP_EXECUTE
    WORLD_WRITE WORLD_READ WORLD_EXECUTE)
install(FILES ${CMAKE_BINARY_DIR}/hyprcursor.pc DESTINATION ${CMAKE_INSTALL_LIBDIR}/pkgconfig)
