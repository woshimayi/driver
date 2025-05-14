cmake_minimum_required(VERSION 3.28)

project(clion_test)
set(CMAKE_CXX_STANDARD 20)

set(CMAKE_INSTALL_PREFIX "${CMAKE_SOURCE_DIR}/install")

# 设置不同配置的输出目录
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG "${CMAKE_BINARY_DIR}/debug/bin")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE "${CMAKE_BINARY_DIR}/release/bin")

set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_DEBUG "${CMAKE_BINARY_DIR}/debug/lib")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_RELEASE "${CMAKE_BINARY_DIR}/release/lib")

message("output" ${CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG})
message("output" ${CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE})


add_executable(clion_test main.cpp
        test.cpp
        test.h
        src/mod.cpp
        inc/mod.h
)

# 不同配置的编译选项
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -O0 -g")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O3 -s -DNDEBUG")

include_directories(inc)

add_library(new_target SHARED test.h test.cpp)

add_subdirectory(sub_test)

message("0000" ${CMAKE_BINARY_DIR})
message(STATUS "Current build type: ${CMAKE_BUILD_TYPE}")

install(TARGETS clion_test
        CONFIGURATIONS Debug
        RUNTIME DESTINATION debug/bin
)

install(TARGETS clion_test
        CONFIGURATIONS Release
        RUNTIME DESTINATION release/bin
)

install(TARGETS new_target
        CONFIGURATIONS Debug
        RUNTIME DESTINATION debug/lib
)

install(TARGETS new_target
        CONFIGURATIONS Release
        RUNTIME DESTINATION release/lib
)


message("2222" ${CMAKE_ADDR2LINE})
message("1111" ${CMAKE_ADSP_ROOT})
message("3333" ${clion_test_BINARY_DIR})
message("4444" ${CMAKE_INSTALL_PREFIX})
