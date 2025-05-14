cmake_minimum_required(VERSION 3.28)
project(sub_test)

set(CMAKE_CXX_STANDARD 20)

add_executable(sub_test main.cpp
        sub_1.cpp)


message("zzzzz"  ${CMAKE_SOURCE_DIR})
message("xxxxx"  ${CMAKE_CURRENT_SOURCE_DIR})