cmake_minimum_required (VERSION 0.1)
project (Hello)

# The version number
set (Hello_VERSION_MAJOR 1) # 添加主版本号
set (Hello_VERSION_MINOR 0) # 添加子版本号

# configure a header file to pass some of the CMake settings
# to the source code
configure_file (
    "${PROJECT_SOURCE_DIR}/HelloConfigure.h.in" # 头文件
    "${PROJECT_BINARY_DIR}/HelloConfigure.h"    # 生成的头文件
)

# add the executable
add_executable(Hello Hello.c)       # 添加执行文件 源文件

