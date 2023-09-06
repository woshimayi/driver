cmake_minimum_required(VERSION 3.0.0)
project(call_cmake_test VERSION 0.1.0 LANGUAGES C)

enable_testing()

add_executable(call_cmake_test main.c vos_log.c call_function.c)

MESSAGE(STATUS "This is BINARY dir " ${PROJECT_NAME})       # 构建打印信息
MESSAGE(STATUS "This is BINARY dir ${PROJECT_NAME}")        # 构建打印信息

set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
