cmake_minimum_required (VERSION 0.1)

set (project_name Hello)

project (${project_name})

set (${project_name}_VERSION_MAJOR 0)
set (${project_name}_VERSION_MINOR 1)

message(STATUS ${PROJECT_NAME})
message(STATUS ${PROJECT_VERSION})
message(STATUS "saddsdf")


# should we use our own math functions?
option (USE_MYFUNC "Use ${project_name} provided my function implementtation" ON)


# configure a header file to pass some of the CMake settings
# to the source code
configure_file (
    "${PROJECT_SOURCE_DIR}/${project_name}Configure.h.in"
    "${PROJECT_BINARY_DIR}/${project_name}Configure.h"
)


# add the binary tree to the search path for include files 
# so the we will find ${project_name}Configure
include_directories ("${PROJECT_SOURCE_DIR}")


if (USE_MYFUNC)
    include_directories ("${PROJECT_SOURCE_DIR}/myfunction")
    add_subdirectory(myfunction)
    set (EXTRA_LIBS ${EXTRA_LIBS} myfunction)
endif ()




target_include_directories(
    myfunction
    INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}
    PRIVATE ${CMAKE_CURRENT_BINBARY_DIR}
)


# add the executable
add_executable(${project_name} ${project_name}.c)
target_link_libraries (${project_name} ${EXTRA_LIBS})


# default add executable to /usr/local/bin
# default add head files to /usr/local/include
install (TARGETS ${project_name} DESTINATION bin)
install (FILES "${PROJECT_BINARY_DIR}/${project_name}Configure.h" DESTINATION include)




# union test
include(CTest)

add_test(${project_name}Runs ${project_name})



