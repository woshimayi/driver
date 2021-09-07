cmake_minimum_required(VERSION 3.0.0)
project(cmake_test VERSION 0.1.0)

include(CTest)
enable_testing()

aux_source_directory(src SRC)
include_directories(./src)

message(status "src " ${SRC})

add_executable(cmake_test main.c ${SRC})

set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)
