cmake_minimum_required (VERSION 0.1)
project (Hello)

# The version number
set (Hello_VERSION_MAJOR 1)
set (Hello_VERSION_MINOR 0)

# configure a header file to pass some of the CMake settings
# to the source code
configure_file (
    "${PROJECT_SOURCE_DIR}/HelloConfigure.h.in"
    "${PROJECT_BINARY_DIR}/HelloConfigure.h"
)

# add the executable
add_executable(Hello Hello.c)

