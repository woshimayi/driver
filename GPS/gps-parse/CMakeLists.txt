# cmake_minimum_required(VERSION <specify CMake version here>)
project(market C)

set(CMAKE_C_STANDARD 99)

set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -g3 -Wall")

add_executable(market main.c gps.h gps.c)