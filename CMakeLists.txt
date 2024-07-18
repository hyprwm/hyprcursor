cmake_minimum_required(VERSION 3.19)

file(READ "${CMAKE_SOURCE_DIR}/VERSION" VER_RAW)
string(STRIP ${VER_RAW} HYPRCURSOR_VERSION)

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
pkg_check_modules(deps REQUIRED IMPORTED_TARGET hyprlang>=0.4.2 libzip cairo librsvg-2.0 tomlplusplus)

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
    target_compile_options(hyprcursor PUBLIC
        $<$<COMPILE_LANGUAGE:CXX>:-std=gnu++2b -D__cpp_concepts=202002L>
        -Wno-builtin-macro-redefined)
endif()

# hyprcursor-util
file(GLOB_RECURSE UTILSRCFILES CONFIGURE_DEPENDS "hyprcursor-util/src/*.cpp" "include/hyprcursor/hyprcursor.hpp" "include/hyprcursor/hyprcursor.h" "include/hyprcursor/shared.h")
add_executable(hyprcursor-util ${UTILSRCFILES})
target_include_directories(hyprcursor-util
    PUBLIC "./include"
    PRIVATE "./libhyprcursor" "./hyprcursor-util/src"
)
target_link_libraries(hyprcursor-util PkgConfig::deps hyprcursor)

# tests
add_custom_target(tests)

add_executable(hyprcursor_test1 "tests/full_rendering.cpp")
target_link_libraries(hyprcursor_test1 PRIVATE hyprcursor)
add_test(NAME "Test libhyprcursor in C++ (full rendering)" WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}/tests COMMAND hyprcursor_test1)
add_dependencies(tests hyprcursor_test1)

add_executable(hyprcursor_test2 "tests/only_metadata.cpp")
target_link_libraries(hyprcursor_test2 PRIVATE hyprcursor)
add_test(NAME "Test libhyprcursor in C++ (only metadata)" WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}/tests COMMAND hyprcursor_test2)
add_dependencies(tests hyprcursor_test2)

add_executable(hyprcursor_test_c "tests/c_test.c")
target_link_libraries(hyprcursor_test_c PRIVATE hyprcursor)
add_test(NAME "Test libhyprcursor in C" WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}/tests COMMAND hyprcursor_test_c)
add_dependencies(tests hyprcursor_test_c)

# Installation
install(TARGETS hyprcursor)
install(TARGETS hyprcursor-util)
install(DIRECTORY "include/hyprcursor" DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})
install(FILES ${CMAKE_BINARY_DIR}/hyprcursor.pc DESTINATION ${CMAKE_INSTALL_LIBDIR}/pkgconfig)
