ADD_EXECUTABLE(curltest main.c)

INCLUDE_DIRECTORIES(/usr/include)
TARGET_LINK_LIBRARIES(curltest curl)