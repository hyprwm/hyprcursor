cmake_minimum_required(VERSION 3.19)

project(
    hyprcursor-util
    DESCRIPTION "A utility for creating and converting hyprcursor themes"
)

find_package(PkgConfig REQUIRED)
pkg_check_modules(deps REQUIRED IMPORTED_TARGET hyprlang>=0.4.0 libzip)
add_compile_definitions(HYPRCURSOR_VERSION="${HYPRCURSOR_VERSION}")

file(GLOB_RECURSE SRCFILES CONFIGURE_DEPENDS "src/*.cpp")

set(CMAKE_CXX_STANDARD 23)

add_executable(hyprcursor-util ${SRCFILES})

target_link_libraries(hyprcursor-util PkgConfig::deps)
target_include_directories(hyprcursor-util
PRIVATE
    .
)

install(TARGETS hyprcursor-util)