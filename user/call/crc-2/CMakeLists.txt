# cmake_minimum_required(VERSION <specify CMake version here>)
project(crc C)
set(CMAKE_C_STANDARD 99)

add_executable(crc16.c crc16.h crc32.c crc32.h crc64.c crc64.h)