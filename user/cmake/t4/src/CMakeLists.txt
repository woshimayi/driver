ADD_EXECUTABLE(main main.c)


INCLUDE_DIRECTORIES(/usr/local/include/hello)
TARGET_LINK_LIBRARIES(main libhello.so)					# 添加临时环境变量  export LD_LIBRARY_PATH=/usr/local/lib:$LD_LIBRARY_PATH
