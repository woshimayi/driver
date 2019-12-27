cmake_minimum_required (VERSION 0.1)
project (Hello)

# The version number
set (Hello_VERSION_MAJOR 1)
set (Hello_VERSION_MINOR 0)

# should we use our own math functions?
option (USE_MYFUNC "Use Hello provided my function implementtation" ON)

# configure a header file to pass some of the CMake settings
# to the source code
configure_file (
    "${PROJECT_SOURCE_DIR}/HelloConfigure.h.in"
    "${PROJECT_BINARY_DIR}/HelloConfigure.h"
)


# add the binary tree to the search path for include files 
# so the we will find HelloConfigure
include_directories ("${PROJECT_SOURCE_DIR}")

if (USE_MYFUNC)
    include_directories ("${PROJECT_SOURCE_DIR}/myfunction")
    add_subdirectory(myfunction)
    set (EXTRA_LIBS ${EXTRA_LIBS} myfunction)
endif ()



# add the executable
add_executable(Hello Hello.c)
target_link_libraries (Hello ${EXTRA_LIBS})

