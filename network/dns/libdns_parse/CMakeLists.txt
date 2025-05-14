cmake_minimum_required(VERSION 3.10.0)
project(dns_parse VERSION 0.1.0 LANGUAGES C)

add_library(dns_parse SHARED dns_parse.c)

add_executable(dns_parse_test test.c)

target_link_libraries(dns_parse_test dns_parse)

include(CTest)
add_test(NAME dns_parse_test COMMAND dns_parse_test)

