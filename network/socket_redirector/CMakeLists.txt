cmake_minimum_required(VERSION 3.28.3)
project(untitled C)

set(CMAKE_C_STANDARD 11)

add_executable(socket_redirector socket_redirector.c)

add_executable(my_echo_server my_echo_server.c)

add_executable(my_date_server my_date_server.c)
