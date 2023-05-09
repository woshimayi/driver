cmake_minimum_required (VERSION 0.1)
project (tcping)

# The version number
set (tcping_VERSION_MAJOR 1) # 添加主版本号
set (tcping_VERSION_MINOR 0) # 添加子版本号

# configure a header file to pass some of the CMake settings
# to the source code
configure_file (
    "${PROJECT_SOURCE_DIR}/tcpingConfigure.h.in" # 头文件
    "${PROJECT_BINARY_DIR}/tcpingConfigure.h"    # 生成的头文件
)

# add the executable
add_executable(tcping tcping.c)       # 添加执行文件 源文件

