cmake_minimum_required (VERSION 0.1)

set (project_name net_test)

project (${project_name})

set (VERSION_MAJOR 0)
set (VERSION_MINOR 1)

message(STATUS ${PROJECT_NAME})
message(STATUS ${PROJECT_VERSION})
message(STATUS "saddsdf")

# should we use our own math functions?
option (SRC_FUNC "Use ${project_name} provided my function implementtation" ON)

# configure a header file to pass some of the CMake settings
# to the source code
configure_file (
    "${PROJECT_SOURCE_DIR}/Configure.h.in"
    "${PROJECT_BINARY_DIR}/Configure.h"
)


# add the binary tree to the search path for include files 
# so the we will find ${project_name}Configure
include_directories ("${PROJECT_SOURCE_DIR}")

if (SRC_FUNC)
    include_directories ("${PROJECT_SOURCE_DIR}/src")
    add_subdirectory(src)
    set (EXTRA_LIBS ${EXTRA_LIBS} src)
endif ()


# add the executable
add_executable(${project_name} ${project_name}.c)
target_link_libraries (${project_name} ${EXTRA_LIBS})


# default add executable to /usr/local/bin
# default add head files to /usr/local/include
install (TARGETS ${project_name} DESTINATION bin)
install (FILES "${PROJECT_BINARY_DIR}/Configure.h" DESTINATION include)
